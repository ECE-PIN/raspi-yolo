#ifndef DISPLAY_HANDLER_H
#define DISPLAY_HANDLER_H

#include <sqlite3.h>
#include <zmqpp/zmqpp.hpp>

#include "../../endpoints.h"
#include "../../food_item.h"
#include "../../logger.h"
#include "engine_state.h"

#include <iostream>

class DisplayHandler {
public:
  static DisplayHandler& getInstance() {
    if (s_instance == nullptr) {
      static zmqpp::context default_context;
      static DisplayHandler default_instance(default_context);
      s_instance = &default_instance;
    }
    return *s_instance;
  }

  static void init(const zmqpp::context& context) {
    static DisplayHandler instance(context);
    s_instance = &instance;
  }

  FoodItem detectionSuccess();
  EngineState startToHardware();
  void scanCancelledToVision();
  void zeroWeightChoiceToHardware(const std::string& zeroWeightChoice);
  EngineState checkDetectionResults(EngineState currentState);
  void ignoreVision();

private:
  DisplayHandler();
  DisplayHandler(const zmqpp::context& context);

  Logger logger;

  zmqpp::socket requestHardwareSocket;
  zmqpp::socket requestVisionSocket;
  zmqpp::socket replySocket;

  static DisplayHandler* s_instance;

  // Singleton
  DisplayHandler(const DisplayHandler&)            = delete;
  DisplayHandler& operator=(const DisplayHandler&) = delete;
  DisplayHandler(DisplayHandler&&)                 = delete;
  DisplayHandler& operator=(DisplayHandler&&)      = delete;

  std::string sendMessage(const std::string& message, const std::string& endpoint);
  std::string receiveMessage(const std::string& response, const int timeout);
};

#endif
