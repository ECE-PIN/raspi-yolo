#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include <glog/logging.h>
#include <thread>

#include "../../endpoints.h"
#include "../../food_item.h"
#include "../../logger.h"
#include "ModelHandler.h"
#include "helperFunctions.h"

class ImageProcessor {
public:
  explicit ImageProcessor(zmqpp::context& context);
  void process();
  bool analyze();
  struct FoodItem& getFoodItem();
  void setFoodItem(struct FoodItem&);
  void processImagePair(int, ClassifyObjectReturn&);

private:
  Logger logger;

  zmqpp::socket requestHardwareSocket;
  zmqpp::socket requestDisplaySocket;
  zmqpp::socket replySocket;

  ModelHandler modelHandler;
  FoodItem foodItem;

  const int MAX_IMAGE_COUNT = 16;

  void detectionSucceeded();
  void detectionFailed();

  void foodItemToDisplay();
  void stopHardware();
};

#endif
