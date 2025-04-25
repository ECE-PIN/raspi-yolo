#include "../include/ImageProcessor.h"

/**
 * @param context The zeroMQ context with which to creates with
 * @param externalEndpoints Endpoints to the main components of the system (vision,
 * hardware, and display)
 */
ImageProcessor::ImageProcessor(zmqpp::context& context, std::string& serverAddress)
    : logger("image_processor.txt"), cancelRequested(false),
      requestHardwareSocket(context, zmqpp::socket_type::request),
      requestDisplaySocket(context, zmqpp::socket_type::request),
      modelHandler(context, serverAddress) {
  try {
    this->requestHardwareSocket.connect(ExternalEndpoints::hardwareEndpoint);
    this->requestDisplaySocket.connect(ExternalEndpoints::displayEndpoint);
  } catch (const zmqpp::exception& e) {
    LOG(FATAL) << e.what();
  }
}

/**
 * Parent method to all image processing and analyzing. Initializes the process of
 * identifying a food item.
 *
 * @param None
 * @return None
 */
void ImageProcessor::process() {
  this->logger.log("Entering process");

  this->logger.log("Opening directory: " + foodItem.getImagePath().string());
  if (!isValidDirectory(foodItem.getImagePath())) {
    LOG(FATAL) << "Failed to open image directory";
    return;
  }

  AnalyzeObjectReturn detectedFoodItem = analyze();

  switch (detectedFoodItem) {
  case AnalyzeObjectReturn::Success:
    detectionSucceeded();
    break;
  case AnalyzeObjectReturn::Failure:
    detectionFailed();
    break;
  case AnalyzeObjectReturn::Cancel:
    this->logger.log("Detection of food item cancelled");
    stopHardware();
    break;
  default:
    break;
  }
}

/**
 * Conduct the analyzing and model processing.
 *
 * @param None
 * @return Whether FoodItem is successfully identified
 */
AnalyzeObjectReturn ImageProcessor::analyze() {
  this->logger.log("Entering analyze");
  ClassifyObjectReturn classifyObjectReturn{false, false};
  this->logger.log("Beginning image processing.");
  for (int i = 0; i < 7; i++) {
    if (isCancelRequested()) {
      logger.log("Cancel requested — exiting early.");
      return AnalyzeObjectReturn::Cancel;
    }

    processImagePair(i, classifyObjectReturn);

    if (classifyObjectReturn.foodItem && classifyObjectReturn.expirationDate) {
      this->logger.log(
          "Both food item and expiration date detected. Returning to processor.");
      std::filesystem::path imagePath =
          foodItem.getImagePath() / (std::to_string(i) + ".jpg");
      this->foodItem.setImagePath(std::filesystem::absolute(imagePath));
      return AnalyzeObjectReturn::Success;
    }
  }
  return AnalyzeObjectReturn::Failure;
}

void ImageProcessor::processImagePair(int currentImageNumber,
                                      ClassifyObjectReturn& classifyObjectReturn) {
  this->logger.log("Entering processImagePair");
  std::filesystem::path imageDir = foodItem.getImagePath();

  // Construct expected filenames
  std::filesystem::path topImage =
      imageDir / (std::to_string(currentImageNumber) + "_top.jpg");
  std::filesystem::path sideImage =
      imageDir / (std::to_string(currentImageNumber) + "_side.jpg");
  this->logger.log("Looking for image: " + topImage.string());
  this->logger.log("Looking for image: " + sideImage.string());
  bool topExists  = false;
  bool sideExists = false;

  while (!(topExists && sideExists)) { // Wait until BOTH images exist
    topExists  = std::filesystem::exists(topImage);
    sideExists = std::filesystem::exists(sideImage);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  ClassifyObjectReturn sideImageReturn =
      this->modelHandler.classifyObject(sideImage, this->foodItem);
  ClassifyObjectReturn topImageReturn =
      this->modelHandler.classifyObject(topImage, this->foodItem);

  if (sideImageReturn.foodItem || topImageReturn.foodItem) {
    this->logger.log("Food item found.");
    classifyObjectReturn.foodItem = true;
  }
  if (sideImageReturn.expirationDate || topImageReturn.expirationDate) {
    this->logger.log("Expiration date found.");
    classifyObjectReturn.expirationDate = true;
  }
}

struct FoodItem& ImageProcessor::getFoodItem() { return this->foodItem; }
void ImageProcessor::setFoodItem(struct FoodItem& foodItem) { this->foodItem = foodItem; }

void ImageProcessor::detectionSucceeded() {
  this->logger.log("Item successfully detected");
  stopHardware();
  foodItemToDisplay();
}

void ImageProcessor::detectionFailed() {
  this->logger.log("Item not successfully detected");

  this->logger.log("Sending detection failed to display");
  this->requestDisplaySocket.send(Messages::ITEM_DETECTION_FAILED);
  std::string response;
  this->requestDisplaySocket.receive(response);

  if (response == Messages::AFFIRMATIVE) {
    this->logger.log("Display received failure message");
  }
  else {
    LOG(FATAL) << "Received invalid message from display";
  }
}

void ImageProcessor::foodItemToDisplay() {
  this->logger.log("Indicating detection success to display");
  this->requestDisplaySocket.send(Messages::ITEM_DETECTION_SUCCEEDED);
  this->logger.log("Sent success message to display, awaiting response");
  std::string response;
  this->requestDisplaySocket.receive(response);
  if (response == Messages::AFFIRMATIVE) {
    this->logger.log("Display acknowledged success, sending detected food item: ");
    this->foodItem.logToFile(this->logger);
    response = sendFoodItem(this->requestDisplaySocket, this->foodItem);
    if (response == Messages::AFFIRMATIVE) {
      this->logger.log("Detected food item sent successfully");
    }
    else {
      LOG(FATAL) << "Detected food item transmission failure";
    }
  }
  else {
  }
}

void ImageProcessor::stopHardware() {
  bool hardwareStopping = false;
  while (hardwareStopping == false) {
    this->logger.log("Sending stop signal hardware");
    this->requestHardwareSocket.send(Messages::ITEM_DETECTION_SUCCEEDED);
    this->logger.log("Sent stop signal to hardware");

    std::string response;
    this->requestHardwareSocket.receive(response);

    if (response == Messages::AFFIRMATIVE) {
      this->logger.log("Hardware received stop signal");
      hardwareStopping = true;
    }
    else if (response == Messages::RETRANSMIT) {
      this->logger.log("Hardware requested retransmission");
    }
    else {
      this->logger.log("Received invalid message from hardware: " + response);
      LOG(FATAL) << "Received invalid message from hardware: " << response;
    }
  }
}

void ImageProcessor::requestCancel() { cancelRequested = true; }

void ImageProcessor::resetCancel() { cancelRequested = false; }

bool ImageProcessor::isCancelRequested() { return cancelRequested.load(); }

void ImageProcessor::notifyServer(const std::string& notification) {
  this->modelHandler.notifyServer(notification);
}
