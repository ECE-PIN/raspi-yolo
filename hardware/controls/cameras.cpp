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
#define IMAGE_DIR "/home/geromy/Desktop/Project/raspi-yolo/images/"

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
    execl("/usr/bin/libcamera-still", "libcamera-still", "--width", "4056", "--height",
          "3040", "--nopreview", "--autofocus-on-capture", "on", "--autofocus-speed",
          "fast",
          //      "--autofocus-rang", "full",   // Full autofocus range
          "--exposure", "normal", "--output", filePath.c_str(), // Save location
          "--timeout", "50", (char*)NULL);

    std::cerr << "Error: Failed to execute libcamera-still" << std::endl;
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
          "--exposure", "normal", "--output", filePath.c_str(), // Save location
          "--timeout", "50", (char*)NULL);

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
bool takePhoto(int angle) {
  std::string fileName =
      "/home/geromy/Desktop/Project/raspi-yolo/images/temp/IMG_2002.JPG";

  pid_t pid = fork();
  if (pid == -1) {
    LOG(FATAL) << "Error starting camera process." << strerror(errno);
    return false;
  }
  if (pid == 0) {
    // Reinitialize glog in the child process
    google::ShutdownGoogleLogging();
    google::InitGoogleLogging("takePhotoChild");
    LOG(INFO) << "Capturing at position " << angle;
    char* args[]    = {(char*)"/usr/bin/rpicam-jpeg",
                       (char*)"rpicam-jpeg",
                       (char*)"1920:1080:12:U", // 1920x1080, 12MP, unpacked
                       (char*)"--nopreview",
                       (char*)"--output",
                       (char*)fileName.c_str(), // Save location
                       nullptr};
    const char* arg = "rpicam-jpeg --output "
                      "/home/geromy/Desktop/Project/raspi-yolo/images/temp/test.jpg";
    std::cout << "here" << std::endl;
    // execl("rpicam-jpeg", arg);
    system(arg);
    //      sendDataToVision(pipes.hardwareToVision[WRITE], weight);
    LOG(INFO) << "ERROR: execvp() failed to capture image.";
    exit(1);
  }
  else {
  }
  LOG(INFO) << "Captured image at position " << angle;
  std::cout << "here 2" << std::endl;
  return true;
}
