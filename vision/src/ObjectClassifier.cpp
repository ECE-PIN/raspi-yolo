
#include "../include/ObjectClassifier.h"
#include <iostream>
/**
 * Handle object classification
 *
 * Input:
 * @param imagePath path to the image you wish to extract text from
 * @return success of attempt
 */
bool ObjectClassifier::handleClassification(const std::filesystem::path& imagePath) {
  LOG(INFO) << "Handling EfficientNet Classification";
  std::string result = runModel(imagePath);

  std::string detectedClass = std::string(result);
  if (detectedClass.find("CLASSIFICATION") != std::string::npos) {
    this->foodItem.name           = removePrefix(detectedClass, "CLASSIFICATION: ");
    this->foodItem.category       = FoodCategories::unpackaged;
    this->foodItem.absolutePath   = std::filesystem::absolute(imagePath);
    this->foodItem.expirationDate = calculateExpiration(foodItem.scanDate, foodItem.name);
    this->foodItem.quantity       = 1;
    return true;
  }
  return false;
}

/**
 * Call and return output of python classification script
 *
 * Input:
 * @param imagePath path to the image you wish to extract text from
 * @return string containing detected class
 */
std::string ObjectClassifier::runModel(const std::filesystem::path& imagePath) const {
  LOG(INFO) << "Running EfficientNet model.";

  sendRequest(TaskType::CLS, imagePath);
  std::string result = readResponse();

  if (result.find("ERROR") != std::string::npos) {
    LOG(FATAL) << result;
  }
  LOG(INFO) << result;
  return result;
}

/**
 * Calculate the expiration date of an efficientNet classified item
 *
 * Input:
 * @param scanDate path to the image you wish to extract text from
 * @param foodLabel food label to search and retrieve information for
 * @return the expiration date in std::chrono::year_month_day format
 */
std::chrono::year_month_day ObjectClassifier::calculateExpiration(
    std::chrono::year_month_day scanDate, const std::string& foodLabel) {
  auto foodItemInfo = foodLabels.at(foodLabel);
  // perform addition of days to scan date
  std::chrono::sys_days scanDateDays =
      std::chrono::sys_days(scanDate) + std::chrono::days(foodItemInfo.shelf_life_days);
  return std::chrono::year_month_day{scanDateDays};
}