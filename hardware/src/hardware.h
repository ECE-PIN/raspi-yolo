#ifndef HARDWARE_H
#define HARDWARE_H

#include <zmqpp/zmqpp.hpp>

#include "../../endpoints.h"
#include "../../logger.h"

// TODO Figure out a better way to represent the photo path
//
// There is the constant below IMAGE_DIRECTORY but ../images/temp is also
// used in sendDataToVision. Need a way to better define the photo path.
// Possibly even in main.cpp so that it can be passed to vision as well.

class Hardware {
public:
  Hardware(zmqpp::context& context, const struct ExternalEndpoints& externalEndpoints);

  bool checkStartSignal(int timeoutMs);
  bool startScan();
  void sendDataToVision();

private:
  ExternalEndpoints externalEndpoints;

  Logger logger;

  zmqpp::socket requestVisionSocket;
  zmqpp::socket requestDisplaySocket;
  zmqpp::socket replySocket;

  const std::string IMAGE_DIRECTORY = "/home/pi/Documents/raspi-yolo/images/temp/";

  float itemWeight = 0;

  bool checkWeight();
  void rotateAndCapture();
  bool capturePhoto(int angle);
};

#endif
