#ifndef HARDWARE_H
#define HARDWARE_H

#include <zmqpp/zmqpp.hpp>

#include "../../endpoints.h"
#include "../../food_item.h"
#include "../../logger.h"
class Hardware {
public:
  Hardware(zmqpp::context& context, bool usingMotor, bool usingCamera);

  const char* SerialDevice = "/dev/ttyACM0";
  int baud                 = 9600;
  int arduino_fd           = -1;

  void initDC();
  int initSerialConnection(const char* device, int baudRate);
  float sendCommand(char commandChar);
  bool checkStartSignal(int timeoutMs);
  void sendStartToVision();
  bool startScan();

private:
  Logger logger;

  zmqpp::socket requestVisionSocket;
  zmqpp::socket requestDisplaySocket;
  zmqpp::socket replySocket;

  std::filesystem::path imageDirectory;

  const int MOTOR_IN1 = 23; // GPIO Pin for L298N IN1
  const int MOTOR_IN2 = 24; // GPIO Pin for L298N IN2
  // const int MOTOR_ENA 18; // GPIO Pin forL298N enable (PWM Speed Control)
  float itemWeight = 0;
  char input[10];
  char response[64];
  bool usingMotor;
  bool usingCamera;

  int readLineFromArduino(char* buffer, int maxLen);
  void rotateAndCapture();
  bool takePhotos(int angle);
  void rotateMotor(bool clockwise);
  bool capturePhoto(int angle);
};

#endif
