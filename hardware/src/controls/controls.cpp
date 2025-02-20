#include <fstream>
#include <glog/logging.h>
#include <iostream>
#include <string>
#include <unistd.h>

#include "../../../food_item.h"
#include "../../../pipes.h"
#include "../hardware_entry.h"
#include "../io.h"
#include "cameras.h"
#include "motor.h"
#include "weight.h"

extern Pipes pipes;

/**
 * Rotates platform in 45-degree increments, captures images, and sends the
 * data to the AI Vision system until a full 360-degree rotation is complete.
 * The process can be stopped early if AI Vision sends a "STOP" signal
 * 0 - continue or 1 - stop
 *
 * @return None
 */
void rotateAndCapture(float weight) {
  for (int angle = 0; angle < 8; angle++) {
    /**
     * char filename[] = "IMAGE_DIR + std::to_string(angle) + "_S.jpg --nopreview";
     * takePic(filename);
     * cout << "Captured image at position " << angle << endl;
     */
    LOG(INFO) << "Capturing at position " << angle;
    takePhotos(angle);
    if (angle == 0) {
      sendDataToVision(weight);
    }
    LOG(INFO) << "Rotating platform...";
    rotateMotor();
    usleep(2000);

    bool stopSignal = false;
    if (read(pipes.visionToHardware[READ], &stopSignal, sizeof(stopSignal)) > 0 &&
        stopSignal) {
      LOG(INFO) << "AI Vision identified item. Stopping process.";
      break;
    }
    else {
      LOG(INFO) << "AI Vision did not identify item. Continuing process.";
      continue;
    }
  }
}

/**
 * This is a placeholder function for the redoThis function in hardware_entry.cpp.
 */
void redoThis(struct Pipes pipes) {
  LOG(INFO) << "Sending Images from Hardware to Vision";
  // sendImagesWithinDirectory(pipes.toVision[WRITE], "../images/");

  struct FoodItem foodItem;
  foodItem.photoPath = "../images/apple.jpg";
  foodItem.name      = "Apple";
  const std::chrono::time_point<std::chrono::system_clock> now{
      std::chrono::system_clock::now()};
  foodItem.scanDate       = std::chrono::time_point_cast<std::chrono::days>(now);
  foodItem.expirationDate = std::chrono::time_point_cast<std::chrono::days>(now);
  foodItem.catagory       = "fruit";
  foodItem.weight         = 10.0;
  foodItem.quantity       = 2;

  sendFoodItem(foodItem, pipes.hardwareToVision[WRITE]);
  LOG(INFO) << "Done Sending Images from Hardware to Vision";
}
