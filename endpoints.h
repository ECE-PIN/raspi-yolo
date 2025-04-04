#ifndef ENDPOINTS_H
#define ENDPOINTS_H

#include <string>

struct ExternalEndpoints {
  static const std::string visionEndpoint;
  static const std::string hardwareEndpoint;
  static const std::string displayEndpoint;
};

struct Messages {
  // General indication that a message was received
  static const std::string AFFIRMATIVE;

  // Indicate that the detection of a food item failed for a general reason
  static const std::string ITEM_DETECTION_FAILED;

  // Indiate that the detection of a food item succeeded
  static const std::string ITEM_DETECTION_SUCCEEDED;

  // Send that message again. May not be ready to handle it yet.
  static const std::string RETRANSMIT;

  // Start the scan of a new food item
  static const std::string START_SCAN;

  // Indicate that there is nothing on the platform
  static const std::string ZERO_WEIGHT;

  // There was no weight detected, check again
  static const std::string RETRY;

  // There was no weight detected, override and start a scan anyway
  static const std::string OVERRIDE;

  // There was no weight detected, cancel
  static const std::string CANCEL;

  // Scan was started but cancelled
  static const std::string SCAN_CANCELLED;
};

#endif
