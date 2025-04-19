#include <glog/logging.h>
#include <iostream>
#include <sqlite3.h>
#include <thread>

#include "../../food_item.h"
#include "display_handler.h"
#include "fooditem.pb.h"
#include "sql_food.h"

DisplayHandler* DisplayHandler::s_instance = nullptr;

/**
 * @param context The zeroMQ context with which to create sockets with
 */
DisplayHandler::DisplayHandler(const zmqpp::context& context)
    : logger("display_handler.txt"),
      requestHardwareSocket(context, zmqpp::socket_type::request),
      requestVisionSocket(context, zmqpp::socket_type::request),
      replySocket(context, zmqpp::socket_type::reply) {
  try {
    this->requestHardwareSocket.connect(ExternalEndpoints::hardwareEndpoint);
    this->requestVisionSocket.connect(ExternalEndpoints::visionEndpoint);
    this->replySocket.bind(ExternalEndpoints::displayEndpoint);
  } catch (const zmqpp::exception& e) {
    LOG(FATAL) << e.what();
  }
}

/**
 * Send a message to a zeroMQ endpoint and get back its response.
 *
 * @param message The message to send to the endpoint
 * @param endpoint The zeroMQ endpoint to send the message to
 * @return The response from the endpoint after sending it the message
 */
std::string DisplayHandler::sendMessage(const std::string& message,
                                        const std::string& endpoint) {
  this->logger.log("Entering sendMessage");
  try {
    std::string response;

    if (endpoint == ExternalEndpoints::hardwareEndpoint) {
      this->logger.log("Sending message to hardware: " + message);
      this->requestHardwareSocket.send(message);
      this->logger.log("Message sent to hardware, awaiting response");
      this->requestHardwareSocket.receive(response);
      this->logger.log("Response received: " + response);
      return response;
    }
    else if (endpoint == ExternalEndpoints::visionEndpoint) {
      this->logger.log("Sending message to vision: " + message);
      this->requestVisionSocket.send(message);
      this->logger.log("Message sent to vision, awaiting response");
      this->requestVisionSocket.receive(response);
      this->logger.log("Response received: " + response);
      return response;
    }
    else {
      LOG(FATAL) << "Attempt to send message to invalid endpoint";
    }
  } catch (const zmqpp::exception& e) {
    LOG(FATAL) << "zmqpp error when sending message: " << e.what();
  }
}

/**
 * Receive a zeroMQ message with a specified timeout and send back response if a message
 * is received.
 *
 * @param response What to send back to requesting socket upon receiving a message.
 * @param timeoutMS Max amount of time to wait for a message. If set to 0, return right
 * away if no message.
 * @return The message received. Returns "null" if no message received.
 */
std::string DisplayHandler::receiveMessage(const std::string& response,
                                           const int timeoutMS) {
  try {
    std::string request = "null";
    if (timeoutMS == 0) {
      this->logger.log("Receiving message with no timeout");
      this->replySocket.receive(request, true);
    }
    else {
      this->logger.log("Receiving message with " + std::to_string(timeoutMS) +
                       " ms timeout");
      zmqpp::poller poller;
      poller.add(this->replySocket);

      if (poller.poll(timeoutMS)) {
        if (poller.has_input(this->replySocket)) {
          this->replySocket.receive(request);
          this->logger.log("Received message: " + request);
          this->replySocket.send(response);
        }
      }
    }

    return request;
  } catch (const zmqpp::exception& e) {
    LOG(FATAL) << "zmqpp error when receiving message: " << e.what();
  }
}

/**
 * Handle the success of a scan. Store the identified food item in the database.
 *
 * @param None
 * @return None
 */
FoodItem DisplayHandler::detectionSuccess() {
  this->logger.log("Item detection succeeded, receiving food item");

  FoodItem foodItem;
  bool foodItemReceived = false;
  while (foodItemReceived == false) {
    foodItemReceived =
        receiveFoodItem(this->replySocket, Messages::AFFIRMATIVE, foodItem);
  }
  this->logger.log("Food item received: ");
  foodItem.logToFile(this->logger);

  return foodItem;
}

/**
 * If there is no weight on the platform, the user is given the chance to decide what they
 * want to do. Once the user has decided what they want to do, this function sends their
 * decision to hardware.
 *
 * @param zeroWeightChoice What the user wants to do about no weight detected.
 * @return None
 */
void DisplayHandler::zeroWeightChoiceToHardware(const std::string& zeroWeightChoice) {
  this->logger.log("Sending zero weight choice: " + zeroWeightChoice + " to hardware");

  std::string hardwareResponse =
      sendMessage(zeroWeightChoice, ExternalEndpoints::hardwareEndpoint);

  if (hardwareResponse == Messages::AFFIRMATIVE) {
    this->logger.log(
        "Zero weight choice successfully sent to hardware, switching to scanning state");
  }
  else {
    this->logger.log("Invalid response received from hardware: " + hardwareResponse);
    LOG(FATAL) << "Invalid response received from hardware: " << hardwareResponse;
  }
}

/**
 * Indicate to vision that the user requested that the scan be cancelled.
 *
 * @param None
 * @return None
 */
void DisplayHandler::scanCancelledToVision() {
  this->logger.log("Scan cancelled, sending scan cancelled message to vision");

  std::string visionResponse =
      sendMessage(Messages::SCAN_CANCELLED, ExternalEndpoints::visionEndpoint);

  this->logger.log("Received response from vision");
  if (visionResponse == Messages::AFFIRMATIVE) {
    this->logger.log("Vision received scan cancelled message");
  }
  else {
    this->logger.log("Invalid response received from vision: " + visionResponse);
    LOG(FATAL) << "Invalid response received from vision: " << visionResponse;
  }
}

EngineState DisplayHandler::startToHardware() {
  this->logger.log("Scan initialized, sending start signal to hardware");

  std::string hardwareResponse =
      sendMessage(Messages::START_SCAN, ExternalEndpoints::hardwareEndpoint);

  if (hardwareResponse == Messages::AFFIRMATIVE) {
    this->logger.log("Hardware starting scan");
    return EngineState::SCANNING;
  }
  else if (hardwareResponse == Messages::ZERO_WEIGHT) {
    this->logger.log("Hardware indicated zero weight on platform");
    return EngineState::ZERO_WEIGHT;
  }
  else {
    this->logger.log("Invalid response received from hardware: " + hardwareResponse);
    LOG(FATAL) << "Invalid response received from hardware: " << hardwareResponse;
  }
}

EngineState DisplayHandler::checkDetectionResults(EngineState currentState) {
  const std::string response = Messages::AFFIRMATIVE;
  const int receiveTimeoutMs = 1;
  std::string visionRequest  = receiveMessage(response, receiveTimeoutMs);
  if (visionRequest == "null") {
    return currentState;
  }

  if (visionRequest == Messages::ITEM_DETECTION_SUCCEEDED) {
    this->logger.log("New food item received, switching to scan success state");
    return EngineState::SCAN_SUCCESS;
  }
  else if (visionRequest == Messages::ITEM_DETECTION_FAILED) {
    this->logger.log("Item detection failed, switching to item list state");
    return EngineState::ITEM_LIST;
  }
  else {
    this->logger.log("Invalid request received from vision: " + visionRequest);
    LOG(FATAL) << "Invalid request received from vision: " << visionRequest;
  }
}

void DisplayHandler::ignoreVision() {
  const std::string response = Messages::AFFIRMATIVE;
  const int receiveTimeoutMs = 1;
  std::string request        = receiveMessage(response, receiveTimeoutMs);
  if (request == "null" || request == Messages::ITEM_DETECTION_SUCCEEDED ||
      request == Messages::ITEM_DETECTION_FAILED) {
    ;
  }
}
