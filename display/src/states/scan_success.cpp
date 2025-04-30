#include <iostream>

#include "../log_files.h"
#include "../sql_food.h"
#include "scan_success.h"

/**
 * @param displayGlobal Pass to State
 * @param state Pass to State
 */
ScanSuccess::ScanSuccess(const struct DisplayGlobal& displayGlobal,
                         const EngineState& state)
    : State(displayGlobal, LogFiles::SCAN_SUCCESS, state) {
  this->logger->log("Constructing scan success state");

  const std::string successMessageContent = "Scan successful, is this correct?";
  const SDL_Color successMessageColor     = {0, 255, 0, 255}; // Green
  const SDL_Rect successMessageRectangle  = {0, 100, 0, 0};
  std::unique_ptr<Text> successMessage    = std::make_unique<Text>(
      this->displayGlobal, this->logFile, successMessageRectangle,
      DisplayGlobal::futuramFontPath, successMessageContent, 24, successMessageColor);
  successMessage->setCenteredHorizontal();
  this->rootElement->addElement(std::move(successMessage));

  std::shared_ptr<Button> yesButton =
      std::make_shared<Button>(this->displayGlobal, this->logFile, SDL_Rect{0, 200, 0, 0},
                               "Yes", SDL_Point{0, 0}, [this]() {
                                 this->correctItem  = true;
                                 this->currentState = EngineState::ITEM_LIST;
                               });
  yesButton->setCenteredHorizontal();
  this->rootElement->addElement(yesButton);

  std::shared_ptr<Button> noButton =
      std::make_shared<Button>(this->displayGlobal, this->logFile, SDL_Rect{0, 250, 0, 0},
                               "No", SDL_Point{0, 0}, [this]() {
                                 this->correctItem  = false;
                                 this->currentState = EngineState::ITEM_LIST;
                               });
  noButton->setCenteredHorizontal();
  this->rootElement->addElement(noButton);

  SDL_Rect boundaryRectangle = {0, 150, 600, 30};
  this->scannedItemPanel =
      std::make_shared<Panel>(this->displayGlobal, this->logFile, boundaryRectangle, -1);
  this->scannedItemPanel->setCenteredHorizontal();
  this->scannedItemPanel->addBorder(1);
  this->rootElement->addElement(scannedItemPanel);
}

/**
 * Render
 *
 * @param None
 * @return None
 */
void ScanSuccess::render() const {
  SDL_SetRenderDrawColor(this->displayGlobal.renderer, 0, 0, 0, 255); // Black background
  SDL_RenderClear(this->displayGlobal.renderer);
  this->rootElement->render();
  SDL_RenderPresent(this->displayGlobal.renderer);
}

/**
 * Perform actions upon entering state. Stores the successfully scanned food item to the
 * database.
 *
 * @param None
 * @return None
 */
void ScanSuccess::enter() {
  this->currentState = this->defaultState;
  this->foodItem     = this->displayHandler.detectionSuccess();

  this->logger->log("Storing food item in database");
  sqlite3* database = nullptr;
  openDatabase(&database);
  int id = storeFoodItem(database, foodItem);
  this->foodItem.setId(id);
  sqlite3_close(database);
  this->logger->log("Food item stored in database");

  this->scannedItemPanel->setId(id);
}

/**
 * Perform actions upon exiting state. If the user pressed the no button, then the food
 * item is deleted out of the database.
 *
 * @param None
 * @return None
 */
void ScanSuccess::exit() {
  if (!this->correctItem) {
    deleteById(this->foodItem.getId());
  }
}
