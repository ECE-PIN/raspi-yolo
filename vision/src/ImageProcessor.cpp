#include "../include/ImageProcessor.h"

/**
 * ImageProcessor Constructor. This class handles all image operations.
 * @param pipes Pipe struct for communication with other processes.
 * @param foodItem FoodItem struct for filling out detected information.
 */
ImageProcessor::ImageProcessor(const Pipes& pipes, FoodItem& foodItem)
    : modelHandler(foodItem), pipes(pipes), foodItem(foodItem) {}

/**
 * Parent method to all image processing and analyzing
 */
void ImageProcessor::process() {
  LOG(INFO) << "Vision analyzing all images";
  if (!isValidDirectory(foodItem.imageDirectory)) {
    LOG(FATAL) << "Failed to open image directory" << foodItem.imageDirectory;
    return;
  }
  bool detectedFoodItem = analyze();
  LOG(INFO) << "Successfully analyzed all images";
  if (!detectedFoodItem) {
    LOG(INFO) << "Item not successfully detected";
    // checked 16 images and failed them all
    //  how to handle?
    // Still send whatever information we do have to the display
    writeString(pipes.visionToDisplay[WRITE],
                "Food item not properly identified. Sending incomplete food item.");
  }
  // tell hardware to stop
  LOG(INFO) << "Sent stop signal to hardware";
  bool detectionComplete = true;
  write(pipes.visionToHardware[WRITE], &detectionComplete, sizeof(detectionComplete));

  // send display the food item
  LOG(INFO) << "Sent detected food item to display";
  printFoodItem(foodItem);
  sendFoodItem(foodItem, pipes.visionToDisplay[WRITE]);
}

/**
 * Conduct the analyzing and model processing
 * @return whether FoodItem is successfully identified
 */
bool ImageProcessor::analyze() {
  constexpr int MAX_IMAGE_COUNT = 16;
  int imageCounter              = 0;
  bool objectDetected           = false;
  while (!objectDetected) {
    for (const auto& entry :
         std::filesystem::directory_iterator(foodItem.imageDirectory)) {
      if (toLowerCase(entry.path().extension()) != ".jpg") {
        continue;
      }
      if (++imageCounter > MAX_IMAGE_COUNT) {
        LOG(INFO) << (objectDetected ? "Object expiration date not found"
                                     : "Object not able to be classified");
        return false;
      }
      // check if the object exists in our object classification
      if (!objectDetected) {
        // if object has already been detected no need to run this (still looking for exp
        // date)
        if (this->modelHandler.classifyObject(entry.path())) {
          objectDetected = true;
          return true;
        }
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