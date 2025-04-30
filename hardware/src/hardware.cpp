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
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  // Frequency and pulse break ratio can be configured
  // pinMode(MOTOR_ENA, PWM_MS_OUTPUT);

  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  // pwmWrite(MOTOR_ENA, ###);

  this->logger.log("Motor System Initialized.");
}

/**
 * Initializes the serial connection to the Arduino.
 *
 * @param device The serial device to connect to (e.g., "/dev/ttyACM0").
 * @param baudRate The baud rate for the serial connection (e.g., 9600).
 * @return 0 on success, 1 on failure.
 */
int Hardware::initSerialConnection(const char* device, int baudRate) {
  arduino_fd = serialOpen(device, baudRate);
  this->logger.log("Attempting to open serial device: " + std::string(device));
  if (arduino_fd < 0) {
    this->logger.log("Error: Unable to open serial device " + std::string(device));
    return 1;
  }

  if (wiringPiSetup() == -1) {
    this->logger.log("Error: Unable to initialize wiringPi");
    return 1;
  }

  serialFlush(arduino_fd);
  this->logger.log("Serial device opened successfully: " + std::string(device));
  return 0;
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

          nonzeroWeight = (bool)hardware.sendCommand('4');
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
              this->logger.log("HELP! I SHOULDN'T BE HERE! (INVALID WEIGHT)");
            }
          }
          else {
            receivedRequest = true;
            this->replySocket.send(Messages::AFFIRMATIVE); // Respond to display
            this->logger.log("Weight non zero, getting numeric weight and starting scan");
            this->itemWeight = sendCommand('1') * -1.00f; // weight is negative
            while (itemWeight == -1) {
              int errcnt = 0;
              this->logger.log("Weight error, retrying");
              sendCommand('2');
              this->itemWeight = sendCommand('1') * -1.00f;
              errcnt++;
              if (errcnt == 5) {
                this->logger.log("Error getting weight, setting weight to 9999");
                this->itemWeight = 9999.0f;
                break;
              }
            }
            this->logger.log("Weight received from Arduino: " +
                             std::to_string(this->itemWeight));
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
 * Reads a line from the Arduino serial port.
 *
 * @param buffer The buffer to store the read line.
 * @param maxLen The maximum length of the buffer.
 * @return The number of characters read.
 */
int Hardware::readLineFromArduino(char* buffer, int maxLen) {
  int i       = 0;
  int timeout = 1000; // 1000 ms timeout
  this->logger.log("Reading line from Arduino");
  while (i < maxLen - 1 && timeout--) {
    if (serialDataAvail(arduino_fd)) {
      char c = serialGetchar(arduino_fd);
      if (c == '\n')
        break;
      buffer[i++] = c;
    }
    else {
      usleep(1000); // 1 ms wait
    }
  }
  buffer[i] = '\0';
  this->logger.log("Read line from Arduino: " + std::string(buffer));
  return i;
}

/**
 * Sends a command to the Arduino and reads the response.
 *
 * @param commandChar The command character to send.
 *  Pi 5 sends:
 *  @param 1 -> request for weight
 *  @param 2 -> request for tare, notify of end of process
 *  @param 4 -> request for process start confimation
 * Arduino responds with:
 *  @return 1 -> item weight || -1 for no weight
 *  @return 2 -> 1 for confimation of tare
 *  @return 4 -> 1 - weight present || 0 - no weight
 *  @return The response from the Arduino.
 */
float sendCommand(char commandChar) {
  this->logger.log("Sending command to Arduino: " + std::string(1, commandChar));
  serialFlush(arduino_fd);
  serialPutchar(arduino_fd, commandChar);

  char response[64];
  readLineFromArduino(response, sizeof(response));

  switch (commandChar) {
  case '1':
    {
      this->logger.log("Received weight from Arduino: " + std::string(response));
      float value = atof(response);
      return value;
    }
  case '2':
    {
      this->logger.log("Received tare confirmation from Arduino: " +
                       std::string(response));
      return (float)atoi(response);
    }
  case '4':
    {
      this->logger.log("Received start weight confirmation from Arduino: " +
                       std::string(response));
      return (float)atoi(response);
    }
  default:
    fprintf(stderr, "Unknown command: %c\n", commandChar);
    return -1.0f; // invalid fallback
  }
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
    else {
      this->logger.log("Simulate motor spinning");
      sleep(1);
    }

    // OPTION: use weight to deteremine if item is still present
    // float weight = sendCommand('1');

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
      // shut down motor
      digitalWrite(MOTOR_IN1, LOW);
      digitalWrite(MOTOR_IN2, LOW);
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

  const std::string command0 = cmd0 + np + res + out + topPhoto;
  const std::string command1 = cmd0 + np + res + out + sidePhoto;
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
    digitalWrite(MOTOR_IN1, HIGH);
    digitalWrite(MOTOR_IN2, LOW);
    // pwmWrite(MOTOR_ENA, 255); // Adjust speed
    usleep(900000); // Rotate duration
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, LOW); // HIGH,HIGH || LOW,LOW == off
  }
  else {
    this->logger.log("Rotating counter-clockwise.");
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, HIGH);
    // pwmWrite(MOTOR_ENA, 255);
    usleep(900000); // Rotate duration
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, LOW);
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
