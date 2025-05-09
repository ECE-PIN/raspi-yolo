/*
#ifndef OBJECT_CLASSIFICATION_H
#define OBJECT_CLASSIFICATION_H

#include <filesystem>
#include <glog/logging.h>

#include "../../food_item.h"
#include "IModel.h"

class ObjectClassifier : public IModel {
public:
  ObjectClassifier();

  std::string runModel(const std::filesystem::path& imagePath) const override;
  bool handleClassification(const std::filesystem::path& imagePath) override;

private:
  FoodItem foodItem;
};

#endif
*/
