#ifndef OBJECT_CLASSIFICATION_H
#define OBJECT_CLASSIFICATION_H

#include "../../food_item.h"
#include "../include/foodItemTemplate.h"
#include "IModel.h"
#include <chrono>
#include <filesystem>
#include <glog/logging.h>

class ObjectClassifier : public IModel {
public:
  explicit ObjectClassifier(FoodItem& foodItem) : IModel(foodItem) {}
  std::string runModel(const std::filesystem::path&) const override;
  bool handleClassification(const std::filesystem::path&) override;
  std::chrono::year_month_day calculateExpiration(std::chrono::year_month_day,
                                                  const std::string&);
};

#endif