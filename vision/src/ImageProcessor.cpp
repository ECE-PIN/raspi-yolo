#include "../include/ImageProcessor.h"

/**
 * @param context The zeroMQ context with which to creates with
 * @param externalEndpoints Endpoints to the main components of the system (vision,
 * hardware, and display)
 */
ImageProcessor::ImageProcessor(zmqpp::context& context,
                               const struct ExternalEndpoints& externalEndpoints)
    : externalEndpoints(externalEndpoints),
      requestHardwareSocket(context, zmqpp::socket_type::request),
      requestDisplaySocket(context, zmqpp::socket_type::request),
      replySocket(context, zmqpp::socket_type::reply), modelHandler(context),
      logger("image_processor.txt") {
  try {
    this->requestHardwareSocket.connect(this->externalEndpoints.hardwareEndpoint);
    this->requestDisplaySocket.connect(this->externalEndpoints.displayEndpoint);
    this->replySocket.bind(this->externalEndpoints.visionEndpoint);
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
  this->logger.log("Vision analyzing all images");

  if (!isValidDirectory(foodItem.getImagePath())) {
    std::cerr << "Failed to open image directory" << foodItem.getImagePath();
    return;
  }

  bool detectedFoodItem = analyze();
  this->logger.log("Successfully analyzed all images");
  if (!detectedFoodItem) {
    this->logger.log("Item not successfully detected");
    // checked 16 images and failed them all
    //  how to handle?
    // Still send whatever information we do have to the display
    //    TODO
    //    writeString(pipes.visionToDisplay[WRITE],
    //               "Food item not properly identified. Sending incomplete food item.");
  }

  // Tell hardware to stop
  bool hardwareStopping = false;
  while (hardwareStopping == false) {
    this->logger.log("Sending stop signal to hardware");
    this->requestHardwareSocket.send("item identified");
    this->logger.log("Sent stop signal to hardware");
    std::string response;
    this->requestHardwareSocket.receive(response);
    if (response != "retransmit") {
      hardwareStopping = true;
      this->logger.log("Hardware received stop signal");
    }
    else {
      this->logger.log("Hardware did not receive stop signal, retrying");
    }
  }

  /*
  // TODO
  // send display the food item
  // TODO  printFoodItem(foodItem);
  sendFoodItem(this->requestDisplaySocket, this->foodItem);
  this->logger.log("Sent detected food item to display");
  */
}

/**
 * Conduct the analyzing and model processing.
 *
 * @param None
 * @return Whether FoodItem is successfully identified
 */
bool ImageProcessor::analyze() {
  int imageCounter    = 0;
  bool objectDetected = false;

  while (!objectDetected) {
    for (const auto& entry :
         std::filesystem::directory_iterator(foodItem.getImagePath())) {
      if (toLowerCase(entry.path().extension()) != ".jpg") {
        continue;
      }

      if (++imageCounter > this->MAX_IMAGE_COUNT) {
        if (objectDetected) {
          this->logger.log("Object expiration date not found");
        }
        else {
          this->logger.log("Object not able to be classified");
        }
        return false;
      }

      // Only runs text atm
      if (!objectDetected) {
        // TODO classifyObject always returns true
        if (this->modelHandler.classifyObject(entry.path())) {
          objectDetected = true;
          return true;
        }
      }

      // delete bad image
      try {
        std::filesystem::remove(entry.path());
        this->logger.log("Deleted unclassified image: " + entry.path().string());
      } catch (const std::filesystem::filesystem_error& e) {
        LOG(FATAL) << "Failed to delete image: " << entry.path() << " - " << e.what();
      }

      // check if image directory has new files
      // directory iterator does not update
      // Wait if the directory is empty
      while (!hasFiles(foodItem.getImagePath())) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
      }
    }
  }
  return false;
}

/**
 * Return the food item
 * @return whether FoodItem is successfully identified
 */
struct FoodItem& ImageProcessor::getFoodItem() { return this->foodItem; }

/**
 * set the food item entirely
 */
void ImageProcessor::setFoodItem(struct FoodItem& foodItem) { this->foodItem = foodItem; }
