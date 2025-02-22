#include <glog/logging.h>
#include <iostream>
#include <memory>
#include <sys/select.h>
#include <unistd.h>
#include <vector>

#include "food_item.h"
#include <filesystem>

/**
 * Send a food item struct through a specified pipe. Not all fields have to be populated.
 * Strings are handled differently due to them being std::strings.
 *
 * @param foodItem The food item to send.
 * @param pipeToWrite The pipe to send the food item through.
 * @return None
 */
void sendFoodItem(struct FoodItem foodItem, int pipeToWrite) {
  write(pipeToWrite, &foodItem.id, sizeof(foodItem.id));

  writeString(pipeToWrite, foodItem.absolutePath);
  writeString(pipeToWrite, foodItem.imageDirectory);
  writeString(pipeToWrite, foodItem.name);
  writeString(pipeToWrite, foodCategoryToString(foodItem.category));

  write(pipeToWrite, &foodItem.scanDate, sizeof(foodItem.scanDate));
  write(pipeToWrite, &foodItem.expirationDate, sizeof(foodItem.expirationDate));
  write(pipeToWrite, &foodItem.weight, sizeof(foodItem.weight));
  write(pipeToWrite, &foodItem.quantity, sizeof(foodItem.quantity));
}

/**
 * Receive a food item struct through a pipe. Strings are handled differently due to them
 * being std::strings. Uses select to timeout if no data available on the pipe.
 *
 * @param foodItem A food item struct to store the food item being received. Pass in an
 * empty food item. After function completes, the received food item will be in this
 * struct.
 * @param pipeToRead The pipe to read the food item from.
 * @param timeout How long to check the pipe for before returning.
 * @return True if a food item was received and false if no food item was available on the
 * pipe.
 */
bool receiveFoodItem(struct FoodItem& foodItem, int pipeToRead, struct timeval timeout) {
  fd_set readPipeSet;

  FD_ZERO(&readPipeSet);
  FD_SET(pipeToRead, &readPipeSet);

  // Check pipe for data
  int pipeReady = select(pipeToRead + 1, &readPipeSet, NULL, NULL, &timeout);

  if (pipeReady == -1) {
    LOG(FATAL) << "Select error when receiving food item";
  }
  else if (pipeReady == 0) { // No data available
    return false;
  }
  if (FD_ISSET(pipeToRead, &readPipeSet)) { // Data available
    read(pipeToRead, &foodItem.id, sizeof(foodItem.id));

    foodItem.absolutePath   = readString(pipeToRead);
    foodItem.imageDirectory = readString(pipeToRead);
    foodItem.name           = readString(pipeToRead);
    foodItem.category       = foodCategoryFromString(readString(pipeToRead));

    read(pipeToRead, &foodItem.scanDate, sizeof(foodItem.scanDate));
    read(pipeToRead, &foodItem.expirationDate, sizeof(foodItem.expirationDate));
    read(pipeToRead, &foodItem.weight, sizeof(foodItem.weight));
    read(pipeToRead, &foodItem.quantity, sizeof(foodItem.quantity));
    return true;
  }
  return false;
}

/**
 * Send a string through a pipe. Needed because can't send a std::string directly.
 *
 * @param pipeToWrite Pipe to send the string through.
 * @param stringToSend The string to send through the pipe.
 * @return None
 */
void writeString(int pipeToWrite, const std::string& stringToSend) {
  size_t stringToSendLength = stringToSend.length();
  write(pipeToWrite, &stringToSendLength, sizeof(stringToSendLength));
  write(pipeToWrite, stringToSend.c_str(), stringToSendLength);
}

/**
 * Read a string from a pipe. Needed because std::string needs to be sent as a c style
 * string.
 *
 * @param pipeToRead Pipe to read the string from.
 * @return The string read from the pipe.
 */
std::string readString(int pipeToRead) {
  size_t stringLength;
  read(pipeToRead, &stringLength, sizeof(stringLength));
  char* stringBuffer = new char[stringLength + 1];
  read(pipeToRead, stringBuffer, stringLength);
  stringBuffer[stringLength] = '\0';
  std::string sentString(stringBuffer);
  delete[] stringBuffer;
  return sentString;
}

/**
 * Convert a food category enum to string
 *
 * @param category Enum to convert
 * @return The string version
 */
std::string foodCategoryToString(const FoodCategories& category) {
  switch (category) {
  case FoodCategories::unknown:
    return "unknown";
  case FoodCategories::unpackaged:
    return "unpackaged";
  case FoodCategories::packaged:
    return "packaged";
  default:
    return "invalid";
  }
}

/**
 * Convert a string to food category
 *
 * @param str String to convert
 * @return The enum version
 */
FoodCategories foodCategoryFromString(const std::string& str) {
  if (str == "unknown")
    return FoodCategories::unknown;
  if (str == "unpackaged")
    return FoodCategories::unpackaged;
  if (str == "packaged")
    return FoodCategories::packaged;
  return FoodCategories::unknown; // Default case
}

/**
 * Print out FoodItem
 *
 * @param item food item to print
 */
void printFoodItem(const FoodItem& item) {
  std::cout << "Food Item Details:\n";
  std::cout << "ID: " << item.id << "\n";
  std::cout << "Name: " << item.name << "\n";
  std::cout << "Category: " << static_cast<int>(item.category)
            << "\n"; // Adjust based on enum to string conversion
  std::cout << "Scan Date: " << static_cast<int>(item.scanDate.year()) << "-"
            << static_cast<unsigned>(item.scanDate.month()) << "-"
            << static_cast<unsigned>(item.scanDate.day()) << "\n";
  std::cout << "Expiration Date: " << static_cast<int>(item.expirationDate.year()) << "-"
            << static_cast<unsigned>(item.expirationDate.month()) << "-"
            << static_cast<unsigned>(item.expirationDate.day()) << "\n";
  std::cout << "Weight: " << item.weight << " grams\n";
  std::cout << "Quantity: " << item.quantity << "\n";
  std::cout << "Image Directory: " << item.imageDirectory.string() << "\n";
  std::cout << "Absolute Path: " << item.absolutePath.string() << "\n";
}
