#ifndef DISPLAY_HANDLER_H
#define DISPLAY_HANDLER_H

#include <sqlite3.h>
#include <zmqpp/zmqpp.hpp>

#include "../../endpoints.h"
#include "../../food_item.h"
#include "../../logger.h"

class DisplayHandler {
public:
  DisplayHandler(const zmqpp::context& context);

  std::string sendMessage(const std::string& message, const std::string& endpoint);
  std::string receiveMessage(const std::string& response, const int timeout);

  void detectionSuccess();

private:
  Logger logger;

  zmqpp::socket requestHardwareSocket;
  zmqpp::socket requestVisionSocket;
  zmqpp::socket replySocket;
};

#endif
