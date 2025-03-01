#include <glog/logging.h>
#include <iostream>
#include <sqlite3.h>

#include "../../food_item.h"
#include "../../pipes.h"
#include "display_handler.h"
#include "sql_food.h"

DisplayHandler::DisplayHandler(struct Pipes externalPipes,
                               int* displayToEngine,
                               int* engineToDisplay)
    : externalPipes(externalPipes), displayToEngine(displayToEngine),
      engineToDisplay(engineToDisplay) {}

/**
 * Handle communication between display and the other major components of the system
 * (vision and hardware).
 *
 * @param None
 * @return None
 */
void DisplayHandler::handleExternal() {
  struct timeval timeout;
  timeout.tv_sec  = 1;
  timeout.tv_usec = 0;

  struct FoodItem foodItem;
  bool foodItemReceived = false;
  foodItemReceived =
      receiveFoodItem(foodItem, this->externalPipes.visionToDisplay[READ], timeout);

  if (foodItemReceived) {
    std::cout << "food item received on display" << std::endl;
    LOG(INFO) << "Food item received from vision";
    sqlite3* database = nullptr;
    openDatabase(&database);

    storeFoodItem(database, foodItem);
    writeString(this->displayToEngine[WRITE], "ID successful");

    sqlite3_close(database);
  }
}

/**
 * Handle communication between the main display process and the engine process.
 *
 * @param None
 * @param None
 */
void DisplayHandler::handleEngine() {
  fd_set readPipeSet;

  FD_ZERO(&readPipeSet);
  FD_SET(this->engineToDisplay[READ], &readPipeSet);

  struct timeval timeout;
  timeout.tv_sec  = 1;
  timeout.tv_usec = 0;
  int pipeReady =
      select(this->engineToDisplay[READ] + 1, &readPipeSet, NULL, NULL, &timeout);

  if (pipeReady == -1) {
    LOG(FATAL) << "Select error when receiving food item";
  }
  else if (pipeReady == 0) { // No data available
    ;
  }
  if (FD_ISSET(this->engineToDisplay[READ], &readPipeSet)) { // Data available
    sendStartSignal();
  }
}

/**
 * Signify to hardware that the user would like to scan in an item.
 *
 * @param None
 * @return None
 */
void DisplayHandler::sendStartSignal() {
  LOG(INFO) << "Sending start signal to hardware";
  std::string sdlString = readString(this->engineToDisplay[READ]);
  writeString(this->externalPipes.displayToHardware[WRITE], sdlString);
}
