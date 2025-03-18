#include "../include/OCRResult.h"

OCRResult::OCRResult() {}

std::vector<std::string> OCRResult::getExpirationDates() { return this->expirationDates; }
std::vector<std::string> OCRResult::getFoodItems() { return this->foodItems; }
void OCRResult::setExpirationDates(std::vector<std::string> expirationDates) {
  this->expirationDates = expirationDates;
}
void OCRResult::setFoodItems(std::vector<std::string> foodItems) {
  this->foodItems = foodItems;
}