
#include <filesystem>
#include <fstream>
#include <glog/logging.h>
#include <iostream>
#include <string>
#include <unistd.h>

#include "../../food_item.h"
#include "../../pipes.h"
#include "../controls/cameras.h"
// #include "../controls/weight.h"
#include "hardware_entry.h"
#include "io.h"

/*
 * Entry into the hardware code. Only called from main after hardware child process is
 * forked.
 *
 * Input:
 * @param pipes Pipes for hardware to communicate with the other processes
 * @return None
 */
void hardwareEntry(struct Pipes pipes) {
  LOG(INFO) << "Within hardware process";

  // Close write end of read pipes
  close(pipes.displayToHardware[WRITE]);
  close(pipes.visionToHardware[WRITE]);

  // Close read end of write pipes
  close(pipes.hardwareToDisplay[READ]);
  close(pipes.hardwareToVision[READ]);

  /*
   * Wait for start signal from Display with 0.5sec sleep.
   */
  while (1) {
    LOG(INFO) << "Waiting for start signal from Display";
    if (receivedStartSignal(pipes.displayToHardware[READ])) {
      LOG(INFO) << "Checking if there is weight on the platform";
      //      weightSetup();
      //      float weight = getWeight();
      float weight = 1;
      if (weight > 0) {
        LOG(INFO) << "Weight detected on platform. Beginning scan.";
        // send 1 to display to indicate weight
        rotateAndCapture(pipes, weight);
      }
      else {
        LOG(INFO) << "No weight detected on platform.";
        // send 0 to display to indicate no weight
        continue;
      }
    }
    else {
      usleep(500000);
    }
  }
}

/**
 * Rotates platform in 45-degree increments, captures images, and sends the
 * data to the AI Vision system until a full 360-degree rotation is complete.
 * The process can be stopped early if AI Vision sends a "STOP" signal
 * 0 - continue or 1 - stop
 *
 * @return None
 */
void rotateAndCapture(struct Pipes pipes, float weight) {
  for (int angle = 0; angle < 8; angle++) {
    LOG(INFO) << "At location " << angle << " of 8";
    // takePhotos(angle);    // This function is for both cameras
    if (takePhoto(angle) == false) {
      LOG(INFO) << "Error taking photo";
    }; // This function is for one camera/testing

    if (angle == 0) {
      sleep(10);
      sendDataToVision(pipes.hardwareToVision[WRITE], weight);
    }

    LOG(INFO) << "Rotating platform...";
    std::cout << "rotating platform" << std::endl;
    //    rotateMotor();
    usleep(200);

    bool stopSignal = false;
    if (read(pipes.visionToHardware[READ], &stopSignal, sizeof(stopSignal)) > 0 &&
        stopSignal) {
      std::cout << "AI Vision identified item. Stopping process." << std::endl;
      LOG(INFO) << "AI Vision identified item. Stopping process.";
      break;
    }
    else {
      LOG(INFO) << "AI Vision did not identify item. Continuing process.";
      std::cout << "whoops" << std::endl;
    }
  }
}
