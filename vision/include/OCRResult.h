#ifndef OCR_RESULT_H
#define OCR_RESULT_H

#include <string>
#include <vector>

class OCRResult {
private:
  std::vector<std::string> foodItems;
  std::vector<std::string> expirationDates;

public:
  OCRResult();
  inline bool hasFoodItems() const { return !foodItems.empty(); }
  inline bool hasExpirationDates() const { return !expirationDates.empty(); }
  std::vector<std::string> getFoodItems();
  std::vector<std::string> getExpirationDates();
  void setFoodItems(std::vector<std::string>);
  void setExpirationDates(std::vector<std::string>);
};

#endif