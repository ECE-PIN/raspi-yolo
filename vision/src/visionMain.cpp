#include "../include/visionMain.h"

/**
 * Entry into the vision code. Only called from main after vision child process is
 * forked.
 *
 * @param context The zeroMQ context with which to create sockets with
 * @param externalEndpoints Endpoints to the main components of the system (vision,
 * hardware, and display)
 * @return None
 */
void visionEntry(zmqpp::context& context) {
  Logger logger("vision_entry.txt");
  logger.log("Within vision process");

  zmqpp::socket replySocket(context, zmqpp::socket_type::reply);
  replySocket.bind(VisionExternalEndpoints::visionMainEndpoint);

  ServerAddress addresses = connectToServer(logger);

  ImageProcessor processor(context, addresses.serverAddress);
  std::atomic_bool isProcessing{false};

  processor.notifyServer(NotifyServerConstants::connected);

  createListenerThread(context, processor);
  createHeartBeatThread(context, isProcessing, addresses.heartbeatAddress);

  zmqpp::poller poller;
  poller.add(replySocket);

  while (1) {
    FoodItem foodItem;
    logger.log("Waiting for Food Item");

    while (!startSignalCheck(replySocket, logger, foodItem, poller)) {
      ;
    }
    isProcessing = true;
    processor.notifyServer(NotifyServerConstants::start);
    processor.setFoodItem(foodItem);
    processor.process();
    processor.notifyServer(NotifyServerConstants::stop);
    isProcessing = false;

    if (processor.isCancelRequested()) {
      logger.log("Process was canceled by user.");
    }
    processor.resetCancel();
  }
}

/**
 * polls for 1000ms for socket activity and then checks replySocket for input
 *
 * @param replySocket replySocket to check for input on
 * @param logger Logger being used to log visionMain
 * @param foodItem foodItem to return with data
 * @param poller poller to use for polling socket activity
 * @return whether start signal was received or not
 */
bool startSignalCheck(zmqpp::socket& replySocket,
                      const Logger& logger,
                      FoodItem& foodItem,
                      zmqpp::poller& poller) {
  try {
    if (poller.poll(1000)) {
      if (poller.has_input(replySocket)) {
        receiveFoodItem(replySocket, Messages::AFFIRMATIVE, foodItem);
        logger.log("Received start signal: ");
        foodItem.logToFile(logger);
        return true;
      }
      return false;
    }
    else {
      logger.log("Did not receive start signal");
      return false;
    }
  } catch (const zmqpp::exception& e) {
    LOG(FATAL) << e.what();
    return false;
  }
}

/**
 * creates and returns thread to listen to vision endpoint and handle it
 *
 * @param context context
 * @param processor processor item
 */
void createListenerThread(zmqpp::context& context, ImageProcessor& processor) {
  std::thread thread([&context, &processor]() {
    Logger logger("vision_listener.txt");
    logger.log("Within vision listener thread");
    zmqpp::socket listenerSocket(context, zmqpp::socket_type::reply);
    listenerSocket.bind(ExternalEndpoints::visionEndpoint);

    zmqpp::socket messengerSocket(context, zmqpp::socket_type::request);
    messengerSocket.connect(VisionExternalEndpoints::visionMainEndpoint);

    while (true) {
      zmqpp::message msg;
      listenerSocket.receive(msg);

      std::string command;
      msg >> command;

      if (command == Messages::SCAN_CANCELLED) {
        logger.log("Cancel command received.");
        processor.requestCancel();
        processor.stopHardware();
        listenerSocket.send(Messages::AFFIRMATIVE);
        logger.log("Cancel received.");
      }
      else if (command == Messages::START_SCAN) {
        logger.log("Start command received. Notifying hardware of successful receive");
        listenerSocket.send(Messages::AFFIRMATIVE); // Tell hardware to send food item

        zmqpp::message foodItemString;
        logger.log("Waiting to retrieve food item from hardware");
        listenerSocket.receive(foodItemString); // Grab food item
        logger.log("Food item received. Notifying hardware.");
        listenerSocket.send(Messages::AFFIRMATIVE); // Confirm receipt

        logger.log("Forwarding food item to visionMain.");
        messengerSocket.send(foodItemString); // Forward to main
        zmqpp::message response;
        logger.log("Waiting for response back from visionMain.");
        messengerSocket.receive(response); // Wait for ack
        std::string resp;
        response >> resp;
        if (resp != Messages::AFFIRMATIVE) {
          LOG(FATAL) << "Error in sending food item to visionmain.";
        }
        logger.log("Response received.");
      }
    }
  });
  thread.detach();
}

/**
 * creates and returns thread to send heartbeat to server
 *
 * @param context context
 * @param isProcessing bool to track if processing is occuring
 * @param heartbeatAddress string endpoint to connect to
 */
void createHeartBeatThread(zmqpp::context& context,
                           std::atomic_bool& isProcessing,
                           std::string& heartbeatAddress) {
  std::thread thread([&context, &isProcessing, heartbeatAddress]() {
    Logger logger("vision_heartbeat.txt");
    logger.log("Within vision hearbeat thread");
    zmqpp::socket heartbeatSocket(context, zmqpp::socket_type::request);
    heartbeatSocket.connect(heartbeatAddress);

    while (true) {
      if (!isProcessing.load()) {
        try {
          zmqpp::message heartbeat;
          heartbeat << "heartbeat";
          heartbeatSocket.send(heartbeat);

          zmqpp::message response;
          heartbeatSocket.receive(response);
          std::string reply;
          response >> reply;
          logger.log("Heartbeat acknowledged: " + reply);
        } catch (const zmqpp::exception& e) {
          logger.log("Heartbeat error: " + std::string(e.what()));
        }
      }

      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  });
  thread.detach();
}

ServerAddress connectToServer(const Logger& logger) {
  logger.log("in connectToServer");
  std::filesystem::path path = "../vision/config.json";
  Config cfg                 = loadConfig(path);
  ServerAddress addresses{"", ""};
  std::string serverIP = "";

  if (cfg.useEthernet) {
    logger.log("Using static ethernet IP set in" + path.string());
    serverIP = cfg.ethernetIP;
  }
  else {
    logger.log("Attempting server broadcast");
    serverIP = discoverServerViaUDP(logger, cfg.discoveryPort);
  }
  if (!serverIP.empty()) {
    logger.log("Discovered Server IP: " + serverIP);
    std::cout << "Discovered Server IP: " + serverIP << std::endl;
    addresses.serverAddress = "tcp://" + serverIP + ":" + std::to_string(cfg.serverPort);
    addresses.heartbeatAddress =
        "tcp://" + serverIP + ":" + std::to_string(cfg.heartbeatPort);
    logger.log("server address: " + addresses.serverAddress);
    logger.log("server address: " + addresses.heartbeatAddress);
  }
  else {
    LOG(FATAL) << "Failed to find server address.";
  }

  return addresses;
}
