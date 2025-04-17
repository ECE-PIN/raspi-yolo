#include <filesystem>
#include <glog/logging.h>
#include <unistd.h>
#include <wiringPi.h>

#include "../../endpoints.h"
#include "../../food_item.h"
#include "hardware.h"

/**
 * @param context The zeroMQ context with which to creates with
 * @param externalEndpoints Endpoints to the main components of the system (vision,
 * hardware, and display)
 */
Hardware::Hardware(zmqpp::context& context, bool usingMotor, bool usingCamera)
    : logger("hardware_log.txt"),
      requestVisionSocket(context, zmqpp::socket_type::request),
      requestDisplaySocket(context, zmqpp::socket_type::request),
      replySocket(context, zmqpp::socket_type::reply),
      // imageDirectory(std::filesystem::current_path() / "tmp/images/"),
      usingMotor(usingMotor), usingCamera(usingCamera) {
  if (this->usingCamera) {
    this->imageDirectory = std::filesystem::current_path() / "tmp/images/";

    if (!std::filesystem::exists(imageDirectory)) {
      if (std::filesystem::create_directories(imageDirectory)) {
        this->logger.log("Created directory: " + imageDirectory.string());
      }
      else {
        this->logger.log("Failed to create directory: " + imageDirectory.string());
      }
    }
  }
  else {
    this->imageDirectory = std::filesystem::current_path() / "../images/Banana";
  }

  try {
    this->requestVisionSocket.connect(ExternalEndpoints::visionEndpoint);
    this->requestDisplaySocket.connect(ExternalEndpoints::displayEndpoint);
    this->replySocket.bind(ExternalEndpoints::hardwareEndpoint);
  } catch (const zmqpp::exception& e) {
    LOG(FATAL) << e.what();
  }
}

/**
 * Initializes GPIO and sensors.
 *
 * @return None
 */
void Hardware::initDC() {
  // Uses BCM numbering of the GPIOs and directly accesses the GPIO registers.
  wiringPiSetupPinType(WPI_PIN_BCM);

  this->logger.log("Motor System Initialization");
  // Setup DC Motor Driver Pins
  pinMode(23, OUTPUT);
  pinMode(24, OUTPUT);
  // Frequency and pulse break ratio can be configured
  // pinMode(MOTOR_ENA, PWM_MS_OUTPUT);

  digitalWrite(23, LOW);
  digitalWrite(24, LOW);
  // pwmWrite(MOTOR_ENA, ###);

  this->logger.log("Motor System Initialized.");
}

/**
 * Checks for a start signal from the display. Return if have not received a message by
 * the timeout.
 *
 * @param None
 * @return bool - True if start signal received, false otherwise
 */
bool Hardware::checkStartSignal(int timeoutMs) {
  bool receivedRequest = false;
  this->logger.log("Checking for start signal from display");

  /*
  this->logger.log("Checking weight");
  if (checkWeight() == false) {
    // TODO handle no weight on platform
    // Send error to display
    // Possible pattern HW error msg -> Display message with 3 options:
    // 1. Retry 2. Skip/Override 3. Cancel
    // Response from Display will then decide action
    this->logger.log("No weight detected on platform");
    return false;
  }
  */

  try {
    zmqpp::poller poller;
    poller.add(this->replySocket);

    if (poller.poll(timeoutMs)) {
      if (poller.has_input(this->replySocket)) {
        bool nonzeroWeight          = false;
        bool zeroWeightDecisionMade = false;
        while (nonzeroWeight == false && zeroWeightDecisionMade == false) {
          std::string request;
          this->replySocket.receive(request);

          this->logger.log("Received start signal from display, checking weight");

          nonzeroWeight = checkWeight();
          if (nonzeroWeight == false) {
            this->logger.log("Informing display that no weight detected on plaform");
            this->replySocket.send(Messages::ZERO_WEIGHT);
            this->logger.log("Informed display that no weight detected on platform");
            std::string zeroWeightResponse;
            this->logger.log("Waiting for zero weight response from display");
            this->replySocket.receive(zeroWeightResponse);
            this->logger.log("Received zero weight response from display " +
                             zeroWeightResponse);

            if (zeroWeightResponse == Messages::RETRY) {
              this->replySocket.send(Messages::AFFIRMATIVE);
              this->logger.log("Display decided to retry");
              continue;
            }
            else if (zeroWeightResponse == Messages::OVERRIDE) {
              receivedRequest        = true;
              zeroWeightDecisionMade = true;
              this->replySocket.send(Messages::AFFIRMATIVE);
              this->logger.log("Display decided to override zero weight, starting scan");
            }
            else if (zeroWeightResponse == Messages::CANCEL) {
              zeroWeightDecisionMade = true;
              this->replySocket.send(Messages::AFFIRMATIVE);
              this->logger.log("Display decided to cancel, aborting scan");
            }
            else {
              // TODO handle invalid zero weight response
            }
          }
          else {
            receivedRequest = true;
            this->replySocket.send(Messages::AFFIRMATIVE); // Respond to display
            this->logger.log("Weight non zero, starting scan");
          }
        }
      }
    }
    else {
      this->logger.log("Did not receive start signal from display");
    }
    return receivedRequest;
  } catch (const zmqpp::exception& e) {
    LOG(FATAL) << e.what();
  }
}

/**
 * Sends the photo directory and weight data to the AI Vision system.
 *
 * @param weight The weight of the object on the platform.
 * @return None
 */
void Hardware::sendStartToVision() {
  const std::chrono::time_point<std::chrono::system_clock> now{
      std::chrono::system_clock::now()};

  std::chrono::year_month_day scanDate = std::chrono::floor<std::chrono::days>(now);

  FoodItem foodItem(this->imageDirectory, scanDate, this->itemWeight);

  std::string response;
  this->logger.log("Sending start signal to vision: ");
  foodItem.logToFile(this->logger);
  this->requestVisionSocket.send(Messages::START_SCAN);
  this->logger.log("Awaiting ack from vision.");
  this->requestVisionSocket.receive(response);
  if (response != Messages::AFFIRMATIVE) {
    LOG(FATAL) << "ERROR sending start scan to vision";
  }
  this->logger.log("Received ack, sending food item.");
  response = sendFoodItem(this->requestVisionSocket, foodItem);
  if (response == Messages::AFFIRMATIVE) {
    this->logger.log("Successfully sent start signal to vision");
  }
  else {
    LOG(FATAL) << "Error sending start signal to vision";
  }
}

/**
 * Start the scan of a new food item.
 *
 * @param usingCamera True if a raspberry pi camera is connected and can be used to take
 * pictures. False if no camera connected and pictures from images/Apple should be used.
 * @return True if the scan was successful. False if the scan was unsuccessful.
 *
 * TODO Handle no weight on platform
 */
bool Hardware::startScan() {
  this->logger.log("Starting scan");

  if (!this->imageDirectory.empty() && this->usingCamera) {
    this->logger.log("Clearing image directory");
    try {
      for (const auto& entry : std::filesystem::directory_iterator(imageDirectory)) {
        if (entry.is_regular_file()) {
          std::filesystem::remove(entry.path());
          this->logger.log("Deleted image: " + entry.path().string());
        }
      }
    } catch (const std::filesystem::filesystem_error& e) {
      this->logger.log("Error deleting images: " + std::string(e.what()));
    }
  }

  rotateAndCapture();
  this->logger.log("Scan complete");
  return true;
}

/**
 * Read from the weight sensor to get the weight of an object on the platform.
 *
 * @param None
 * @return True if non zero weight. False if zero weight.
 *
 * TODO 1. setup zmqpp with Arduino
 *      2. integrate code to check weight from Arduino Read from weight sensor
 */
bool Hardware::checkWeight() {
  this->itemWeight = 0; // set to 0 for testing
  if (this->itemWeight <= 0) {
    // May need to adjust up because the scale is likely sensitive to vibrations
    return false;
  }
  return true;
}

/**
 * Rotates platform in 45-degree increments, captures images, and sends the
 * data to the AI Vision system until a full 360-degree rotation is complete.
 * The process can be stopped early if AI Vision sends a "STOP" signal
 * 0 - continue or 1 - stop
 *
 * @param None
 * @return None
 *
 * TODO Multiple cameras
 * TODO Rotate motor
 */
void Hardware::rotateAndCapture() {
  for (int angle = 0; angle < 8; angle++) {
    this->logger.log("At location " + std::to_string(angle) + " of 8");

    // Leaving in T/F logic in case we need to add error handling later
    if (this->usingCamera) {
      if (takePhotos(angle) == false) {
        this->logger.log("Error taking photos");
      }
    }

    // Initiate image scanning on server
    if (angle == 0) {
      sendStartToVision();
    }

    if (this->usingMotor) {
      rotateMotor(true);
    }

    // Swap comment lines below if you don't want to wait on realistic motor rotation time
    // usleep(500);
    sleep(3);

    // Last iteration doensn't need to check signal
    if (angle == 7) {
      return;
    }

    this->logger.log("Checking for stop signal from vision");
    bool receivedStopSignal = false;
    bool receivedRequest    = false;
    std::string request;
    receivedRequest = this->replySocket.receive(request, true);
    if (receivedRequest) {
      if (request == Messages::ITEM_DETECTION_SUCCEEDED) {
        this->logger.log("Received stop signal from vision");
        this->replySocket.send(Messages::AFFIRMATIVE);
        receivedStopSignal = true;
      }
      else {
        this->logger.log("Received other from vision: " + request);
        this->replySocket.send(Messages::RETRANSMIT);
      }
    }
    if (receivedStopSignal) {
      this->logger.log("AI Vision identified item. Stopping process.");
      break;
    }
    this->logger.log("AI Vision did not identify item. Continuing process.");
  }
}

/**
 * Takes two photos, one top & side, saves them to the image directory, and logs
 * Note: current "top camera" is ribbon port closer to USB
 *
 * @param int angle - the position of the platform for unique photo ID
 * @return bool - always true
 */
bool Hardware::takePhotos(int angle) {
  this->logger.log("Taking photos at position: " + std::to_string(angle));
  const std::string cmd0 = "rpicam-jpeg --camera 0";
  const std::string cmd1 = "rpicam-jpeg --camera 1";
  const std::string np   = " --nopreview";
  const std::string res  = " --width 4608 --height 2592";
  const std::string out  = " --output ";
  const std::string to = " --timeout 50"; // DO NOT SET TO 0! Will cause infinite preview!
  const std::string topPhoto =
      this->imageDirectory.string() + std::to_string(angle) + "_top.jpg";
  const std::string sidePhoto =
      this->imageDirectory.string() + std::to_string(angle) + "_side.jpg";

  const std::string command0 = cmd0 + np + res + out + topPhoto + to;
  const std::string command1 = cmd1 + np + res + out + sidePhoto + to;
  system(command0.c_str());
  system(command1.c_str());

  this->logger.log("Photos successfully captured at position: " + std::to_string(angle));
  this->logger.log("Exiting takePhotos at angle: " + std::to_string(angle));

  // Always returns true
  return true;
}

/**
 * Controls platform rotation with L298N motor driver.
 * Rotates for a fixed duration.
 * Timing will need to be adjusted.
 * HIGH,LOW == forward LOW,HIGH == backwards
 * @param clockwise Boolean; true for clockwise, false for counterclockwise
 *
 * @return None
 */
// likely needs to be updated after testing to confirm direction
void Hardware::rotateMotor(bool clockwise) {
  this->logger.log("Rotating platform");
  if (clockwise) {
    this->logger.log("Rotating clockwise.");
    digitalWrite(motor_in1, HIGH);
    digitalWrite(motor_in2, LOW);
    // pwmWrite(MOTOR_ENA, 255); // Adjust speed
    usleep(922852); // Rotate duration
    digitalWrite(motor_in1, LOW);
    digitalWrite(motor_in2, LOW); // HIGH,HIGH || LOW,LOW == off
  }
  else {
    this->logger.log("Rotating counter-clockwise.");
    digitalWrite(motor_in1, LOW);
    digitalWrite(motor_in2, HIGH);
    // pwmWrite(MOTOR_ENA, 255);
    usleep(922852); // Rotate duration
    digitalWrite(motor_in1, LOW);
    digitalWrite(motor_in2, LOW);
  }
  this->logger.log("Platform successfully rotated");
}

/**
 * Test Function
 * Captures a photo at the given angle and saves it to the image directory
 *
 * @param int angle - the position of the platform for unique photo ID
 * @return bool - always true
 */
bool Hardware::capturePhoto(int angle) {
  this->logger.log("Capturing photo at position: " + std::to_string(angle));
  std::string fileName =
      this->imageDirectory.string() + std::to_string(angle) + "_test.jpg";

  std::string command = "rpicam-jpeg -n -t 50 1920:1080:12:U --output " + fileName;
  system(command.c_str());

  this->logger.log("Photo successfully captured at position: " + std::to_string(angle));
  this->logger.log("Exiting capturePhoto at angle: " + std::to_string(angle));
  // Always returns true
  return true;
}
