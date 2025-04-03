#include <glog/logging.h>
#include <iostream>
#include <sqlite3.h>
#include <thread>

#include "../../food_item.h"
#include "display_handler.h"
#include "fooditem.pb.h"
#include "sql_food.h"

/**
 * @param context The zeroMQ context with which to create sockets with
 * @param engineEndpoint zeroMQ endpoint to communicate with the display engine
 */
DisplayHandler::DisplayHandler(zmqpp::context& context, const std::string& engineEndpoint)
    : engineEndpoint(engineEndpoint),
      requestEngineSocket(context, zmqpp::socket_type::request),
      requestHardwareSocket(context, zmqpp::socket_type::request),
      requestVisionSocket(context, zmqpp::socket_type::request),
      replySocket(context, zmqpp::socket_type::reply), logger("display_handler.txt") {
  try {
    this->requestEngineSocket.connect(engineEndpoint);
    this->requestHardwareSocket.connect(ExternalEndpoints::hardwareEndpoint);
    this->requestVisionSocket.connect(ExternalEndpoints::visionEndpoint);
    this->replySocket.bind(ExternalEndpoints::displayEndpoint);
  } catch (const zmqpp::exception& e) {
    LOG(FATAL) << e.what();
  }
}

/**
 * Handle all communiation into and out of the display. Check for start signal from
 * engine, send start signal to hardware, then wait for message from vision.
 *
 * @param None
 * @return None
 */
void DisplayHandler::handle() {
  while (true) {
    std::string receivedMessage;
    this->logger.log("Listening for messages");
    this->replySocket.receive(receivedMessage);
    this->logger.log("Received message from socket: " + receivedMessage);

    // Engine wants to start a new scan, decide what to send back to it
    if (receivedMessage == Messages::START_SCAN) {
      std::string startResponse = startSignalToHardware();

      if (startResponse == Messages::AFFIRMATIVE) { // Success, weight on platform
        handleScanStarted();
        // receiveFromVision();
        //  handle scanning
      }
      else if (startResponse == Messages::ZERO_WEIGHT) {
        // Tell engine no weight on platform and wait for next step
        this->logger.log("Indicating to engine that no weight on platform");
        this->replySocket.send(Messages::ZERO_WEIGHT);
        this->logger.log("Indicated to engine that no weight on platform");

        std::string zeroWeightResponse;
        this->replySocket.receive(zeroWeightResponse);

        if (zeroWeightResponse == Messages::RETRY) {
          zeroWeightRetry();
        }
        else if (zeroWeightResponse == Messages::OVERRIDE) {
          zeroWeightOverride();
        }
        else if (zeroWeightResponse == Messages::CANCEL) {
          zeroWeightCancel();
        }
        else {
          LOG(FATAL) << "Unexpected message received: " << receivedMessage;
        }
      }
    }
    else {
      LOG(FATAL) << "Unexpected message received: " << receivedMessage;
    }
  }
}

/**
 * Indicate to hardware that the user has placed an item on the platform and would like to
 * scan it.
 *
 * @param None
 * @return True if hardware started scan. False if hardware not able to start scan.
 */
std::string DisplayHandler::startSignalToHardware() {
  this->logger.log("Received start scan from engine");

  try {
    this->logger.log("Requesting that hardware start scan");
    this->requestHardwareSocket.send(Messages::START_SCAN);

    std::string hardwareResponse;
    this->requestHardwareSocket.receive(hardwareResponse);
    if (hardwareResponse == Messages::AFFIRMATIVE) {
      this->logger.log("Hardware starting scan");
      return Messages::AFFIRMATIVE;
    }
    else if (hardwareResponse == Messages::ZERO_WEIGHT) {
      this->logger.log("Hardware indicated zero weight on platform");
      return Messages::ZERO_WEIGHT;
    }
    else {
      LOG(FATAL) << "Received invalid response from hardware after sending start signal";
    }
  } catch (const zmqpp::exception& e) {
    LOG(FATAL) << e.what();
  }
}

/**
 * Wait for a response from vision indicating success of scan.
 *
 * @param None
 * @return None
 */
void DisplayHandler::handleScanStarted() {
  this->logger.log("Listening for incoming messages while scanning");
  std::string request;
  this->replySocket.receive(request);
  this->logger.log("Message received: " + request);
  if (request == Messages::ITEM_DETECTION_FAILED) {
    detectionFailure();
  }
  else if (request == Messages::ITEM_DETECTION_SUCCEEDED) {
    detectionSuccess();
  }
  else if (request == Messages::SCAN_CANCELLED) {
    scanCancelled();
  }
  else {
    LOG(FATAL) << "Unexpected message received";
  }
}

/**
 * Handle the failure of a scan..
 *
 * @param None
 * @return None
 */
void DisplayHandler::detectionFailure() {
  this->replySocket.send(Messages::AFFIRMATIVE);
  this->logger.log("Item detection failed");

  this->requestEngineSocket.send(Messages::ITEM_DETECTION_FAILED);
  this->logger.log("Failure message send to engine");
  std::string engineResponse;
  this->logger.log("Receiving response from engine");
  this->requestEngineSocket.receive(engineResponse);
  this->logger.log("Received response from engine");
  if (engineResponse == Messages::AFFIRMATIVE) {
    this->logger.log("Engine notified of failure");
  }
  else {
    LOG(FATAL) << "Unexpected message received";
  }
}

/**
 * Handle the success of a scan. Store the identified food item in the database and
 * indicate to the engine that the scan was successful.
 *
 * @param None
 * @return None
 */
void DisplayHandler::detectionSuccess() {
  this->replySocket.send(Messages::AFFIRMATIVE);

  this->logger.log("Item detection succeeded, receiving food item");
  FoodItem foodItem;
  bool foodItemReceived = false;
  while (foodItemReceived == false) {
    foodItemReceived =
        receiveFoodItem(this->replySocket, Messages::AFFIRMATIVE, foodItem);
  }

  this->logger.log("Food item received: ");
  foodItem.logToFile(this->logger);
  this->logger.log("Storing food item in database");
  sqlite3* database = nullptr;
  openDatabase(&database);
  storeFoodItem(database, foodItem);
  sqlite3_close(database);
  this->logger.log("Food item stored in database");

  this->logger.log("Sending success message to engine");
  this->requestEngineSocket.send(Messages::ITEM_DETECTION_SUCCEEDED);
  this->logger.log("Success message send to engine");
  std::string engineResponse;
  this->logger.log("Receiving response from engine");
  this->requestEngineSocket.receive(engineResponse);
  this->logger.log("Received response from engine");
  if (engineResponse == Messages::AFFIRMATIVE) {
    this->logger.log("Item stored in database and engine notified of success");
  }
  else {
    LOG(FATAL) << "Unexpected message received";
  }
}

void DisplayHandler::scanCancelled() {
  this->replySocket.send(Messages::AFFIRMATIVE);

  this->logger.log("Scan cancelled, sending scan cancelled message to vision");
  this->requestVisionSocket.send(Messages::SCAN_CANCELLED);
  this->logger.log("Scan cancelled message sent to vision");
  std::string visionResponse;
  this->logger.log("Receiving response from vision");
  this->requestVisionSocket.receive(visionResponse);
  this->logger.log("Received response from vision");
  if (visionResponse == Messages::AFFIRMATIVE) {
    this->logger.log("Vision received scan cancelled message");
  }
  else {
    LOG(FATAL) << "Unexpected message received";
  }
}

void DisplayHandler::zeroWeightRetry() {
  this->logger.log("Received retry from engine, sending affirmative back");
  // TODO figure out how to retry
  this->replySocket.send(Messages::AFFIRMATIVE);
  this->logger.log("Affirmative sent, sending retry request to hardware");
  this->requestHardwareSocket.send(Messages::RETRY);
  this->logger.log("Sent retry request to hardware");
  std::string retryResponse;
  this->requestHardwareSocket.receive(retryResponse);
  this->logger.log("Hardware received retry request");
}

void DisplayHandler::zeroWeightOverride() {
  this->logger.log("Received override from engine, sending affirmative back");
  this->replySocket.send(Messages::AFFIRMATIVE);
  this->logger.log("Affirmative sent, sending override request to hardware");
  this->requestHardwareSocket.send(Messages::OVERRIDE);
  this->logger.log("Sent override request to hardware");
  std::string overrideResponse;
  this->requestHardwareSocket.receive(overrideResponse);
  // TODO check overrideResponse
  this->logger.log("Hardware received override request, waiting for request from vision");
  handleScanStarted();
}

void DisplayHandler::zeroWeightCancel() {
  this->logger.log("Received cancel from engine, sending affirmative back");
  this->replySocket.send(Messages::AFFIRMATIVE);
  this->logger.log("Affirmative sent, sending cancel request to hardware");
  this->requestHardwareSocket.send(Messages::CANCEL);
  this->logger.log("Sent cancel request to hardware");
  std::string cancelResponse;
  this->requestHardwareSocket.receive(cancelResponse);
  // TODO check cancelResponse
  this->logger.log("Hardware received cancel request");
}
