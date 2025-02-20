#include <fstream>
#include <glog/logging.h>
#include <iostream>
#include <string>
#include <unistd.h>

#include "../../food_item.h"
#include "../../pipes.h"
#include "../src/hardware_entry.h"
#include "../src/io.h"
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
    // takePhotos(angle);    // This function is for both cameras
    takePhoto(angle); // This function is for one camera/testing
    if (angle == 0) {
      sendDataToVision(weight);
    }
    LOG(INFO) << "Rotating platform...";
    rotateMotor();
    usleep(200);

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