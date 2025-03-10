#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <string>

#include "../../log_files.h"
#include "../display_global.h"
#include "../elements/composite_element.h"
#include "main_menu.h"
#include "state.h"

/**
 * @param displayGlobal Global display variables.
 */
MainMenu::MainMenu(struct DisplayGlobal displayGlobal) : logger(LogFiles::mainMenu) {
  this->logger.log("Constructing main menu state");
  this->currentState = EngineState::MAIN_MENU;

  this->displayGlobal        = displayGlobal;
  SDL_Surface* windowSurface = SDL_GetWindowSurface(this->displayGlobal.window);

  SDL_Rect rootRectangle = {0, 0, 0, 0};
  rootRectangle.w        = windowSurface->w;
  rootRectangle.h        = windowSurface->h;
  this->rootElement      = std::make_unique<Container>(rootRectangle);

  // Title
  const char* titleContent    = "Expiration Tracker";
  SDL_Color titleColor        = {0, 255, 0, 255}; // Green
  SDL_Rect titleRect          = {0, 100, 0, 0};
  std::unique_ptr<Text> title = std::make_unique<Text>(
      this->displayGlobal, titleRect, this->displayGlobal.futuramFontPath, titleContent,
      24, titleColor);
  title->setCenteredHorizontal();
  this->rootElement->addElement(std::move(title));

  // Start Scan
  SDL_Rect newScanButtonRectangle       = {200, 150, 200, 50};
  std::unique_ptr<Button> newScanButton = std::make_unique<Button>(
      this->displayGlobal, newScanButtonRectangle, "Scan New Item", SDL_Point{10, 10},
      [this]() { this->currentState = EngineState::SCANNING; }, LogFiles::mainMenu);
  newScanButton->setCenteredHorizontal();
  rootElement->addElement(std::move(newScanButton));

  // View Stored
  SDL_Rect viewStoredButtonRectangle       = {200, 210, 200, 50};
  std::unique_ptr<Button> viewStoredButton = std::make_unique<Button>(
      this->displayGlobal, viewStoredButtonRectangle, "View Stored Items",
      SDL_Point{10, 10}, [this]() { this->currentState = EngineState::ITEM_LIST; },
      LogFiles::mainMenu);
  viewStoredButton->setCenteredHorizontal();
  rootElement->addElement(std::move(viewStoredButton));

  this->logger.log("Main menu state successfully constructed");
}

/**
 * Render all elements.
 *
 * @param None
 * @return None
 */
void MainMenu::render() const {
  SDL_SetRenderDrawColor(this->displayGlobal.renderer, 0, 0, 0, 255); // Black background
  SDL_RenderClear(this->displayGlobal.renderer);
  this->rootElement->render();
  SDL_RenderPresent(this->displayGlobal.renderer);
}
