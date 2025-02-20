#include <fstream>
#include <glog/logging.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * Sample command. rpicam-apps allows bulding customized behavior
 * run libcamera-hello --list-cameras to get camera names
 */
#define IMAGE_DIR "/home/pi/Desktop/Project/raspi-yolo/images/";

/**
 * Takes photos from both camera modules simultaneously using separate processes.
 * Photos should go to "../../images"
 * Image format "image_x_T" || "image_x_S", where x == angle.
 *
 * @param angle - The angle at which the photos are taken. (0-8, where 8==0)
 * @return None
 */
void takePhotos(int angle) {
  pid_t topCam = fork();
  if (topCam == -1) {
    LOG(FATAL) << "Error starting top camera.";
  }
  else if (topCam == 0) {
    std::string filePath = std::string(IMAGE_DIR) + std::to_string(angle) + "_T.jpg";
    execl("/usr/bin/rpicam-still", "rpicam-still", "--width", "4056", "--height", "3040",
          "--nopreview", "--autofocus-on-capture", "on", "--autofocus-speed", "fast",
          //      "--autofocus-rang", "full",   // Full autofocus range
          "--exposure", "noraml", "--output", filePath.c_str(), // Save location
          "--timeout", "500", (char*)NULL);

    std::cerr << "Error: Failed to execute rpicam-still" << std::endl;
    exit(1);
    if (system(filePath.c_str()) == -1) {
      LOG(FATAL) << "Failed to capture image from top camera.";
    }
    exit(0);
  }

  pid_t sideCam = fork();
  if (sideCam == -1) {
    LOG(FATAL) << "Error starting side camera process.";
  }
  else if (sideCam == 0) {
    std::string filePath = std::string(IMAGE_DIR) + std::to_string(angle) + "_T.jpg";
    execl("/usr/bin/rpicam-still", "rpicam-still", "--width", "4056", "--height", "3040",
          "--nopreview", "--autofocus-on-capture", "on", "--autofocus-speed", "fast",
          //      "--autofocus-rang", "full",   // Full autofocus range
          "--exposure", "noraml", "--output", filePath.c_str(), // Save location
          "--timeout", "500", (char*)NULL);

    std::cerr << "Error: Failed to execute rpicam-still" << std::endl;
    exit(1);
    if (system(filePath.c_str()) == -1) {
      LOG(FATAL) << "Failed to capture image from top camera.";
    }
    exit(0);
  }

  waitpid(topCam, NULL, 0);
  waitpid(sideCam, NULL, 0);
  LOG(INFO) << "Photos successful at position " << angle;
}

/**
 * Takes a photo using rpicam-still and saves it to the images directory.
 *
 * @param angle Used to create the fileName for the image.
 * @return None - can add a 0 or -1 for success/failure
 */
void takePhoto(int angle) {
  pid_t pid = fork();
  if (pid == -1) {
    LOG(FATAL) << "Error starting camera process.";
  }
  else if (pid == 0) {
    std::string fileName = std::string(IMAGE_DIR) + std::to_string(angle) + "_test.jpg";
    execl("/usr/bin/rpicam-still", "rpicam-still", "--width", "4056", "--height", "3040",
          "--nopreview", "--autofocus-on-capture", "on", "--autofocus-speed", "fast",
          //      "--autofocus-rang", "full",   // Full autofocus range
          "--exposure", "noraml", "--output", fileName.c_str(), // Save location
          "--timeout", "500", (char*)NULL);

    std::cerr << "Error: Failed to execute rpicam-still" << std::endl;
    if (system(fileName.c_str()) == -1) {
      LOG(FATAL) << "Failed to capture image from camera.";
    }
    exit(0);
  }
  waitpid(pid, NULL, 0);
  LOG(INFO) << "Photo successful at position " << angle;
}