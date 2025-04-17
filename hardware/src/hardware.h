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
  Hardware(zmqpp::context& context, bool usingMotor, bool usingCamera);

  void initDC();
  bool checkStartSignal(int timeoutMs);
  void sendStartToVision();
  bool startScan();

private:
  Logger logger;

  zmqpp::socket requestVisionSocket;
  zmqpp::socket requestDisplaySocket;
  zmqpp::socket replySocket;

  std::filesystem::path imageDirectory;

  float itemWeight    = 0;
  const int motor_in1 = 23; // GPIO Pin for L298N IN1
  const int motor_in2 = 24; // GPIO Pin for L298N IN2
  // const int motor_ena = 18; // GPIO Pin forL298N enable (PWM Speed Control)

  bool usingMotor;
  bool usingCamera;

  bool checkWeight();
  void rotateAndCapture();
  bool takePhotos(int angle);
  void rotateMotor(bool clockwise);
  bool capturePhoto(int angle);
};

#endif
