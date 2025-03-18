#ifndef MODEL_HANDLER_H
#define MODEL_HANDLER_H

#include <filesystem>
#include <glog/logging.h>

#include "../../food_item.h"
#include "../../logger.h"
#include "ObjectClassifier.h"
#include "TextClassifier.h"
#include "externalEndpoints.h"
#include "foodItemList.h"

struct ClassifyObjectReturn {
  bool expirationDate;
  bool foodItem;
};

class ModelHandler {
public:
  ModelHandler(zmqpp::context&);
  ClassifyObjectReturn classifyObject(const std::filesystem::path&, FoodItem&);
  std::chrono::year_month_day extractExpirationDate(const std::string&);
  std::string discoverServerViaUDP();

private:
  Logger logger;

  TextClassifier textClassifier;
};

#endif
