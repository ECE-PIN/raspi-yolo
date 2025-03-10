#include <SDL2/SDL_ttf.h>
#include <glog/logging.h>
#include <iostream>
#include <memory>

#include "../../../../food_item.h"
#include "../../log_files.h"
#include "../../sql_food.h"
#include "../display_global.h"
#include "../elements/dropdown.h"
#include "../elements/panel.h"
#include "../elements/scroll_box.h"
#include "../elements/text.h"
#include "item_list.h"

/**
 * @param displayGlobal Global display variables.
 */
ItemList::ItemList(struct DisplayGlobal displayGlobal) : logger(LogFiles::itemList) {
  this->logger.log("Constructing item list state");
  this->currentState = EngineState::ITEM_LIST;

  this->displayGlobal = displayGlobal;

  this->mediator = std::make_shared<Mediator>(LogFiles::itemList);

  SDL_Surface* windowSurface = SDL_GetWindowSurface(this->displayGlobal.window);
  previousUpdate             = std::chrono::steady_clock::now();

  SDL_Rect rootRectangle = {0, 0, 0, 0};
  rootRectangle.w        = windowSurface->w;
  rootRectangle.h        = windowSurface->h;
  std::cout << rootRectangle.w << std::endl;
  this->rootElement = std::make_shared<Container>(rootRectangle);

  // Scrollbox
  SDL_Rect scrollBoxRect = {0, 0, 400, 100};
  int windowWidth, windowHeight;
  SDL_GetWindowSize(this->displayGlobal.window, &windowWidth, &windowHeight);
  scrollBoxRect.h = windowHeight - 1;
  std::shared_ptr<ScrollBox> scrollBox =
      std::make_shared<ScrollBox>(this->displayGlobal, scrollBoxRect, LogFiles::itemList);

  scrollBox->setPanelHeight(30);
  scrollBox->addBorder(1);
  this->mediator->addScrollBox(scrollBox);
  this->rootElement->addElement(std::move(scrollBox));
  this->rootElement->update();

  // Dropdown
  std::shared_ptr<Dropdown> sortBy =
      std::make_shared<Dropdown>(this->displayGlobal, SDL_Rect{450, 0, 0, 0}, "Sort by:");

  std::shared_ptr<Button> sortByExpirationLowToHigh = std::make_shared<Button>(
      this->displayGlobal, SDL_Rect{0, 0, 0, 0}, "Expiration Date - Lowest to Highest",
      SDL_Point{0, 0}, "low to high", LogFiles::itemList);
  sortByExpirationLowToHigh->initialize();
  this->mediator->addButton(sortByExpirationLowToHigh);

  std::shared_ptr<Button> sortByExpirationHighToLow = std::make_shared<Button>(
      this->displayGlobal, SDL_Rect{0, 0, 0, 0}, "Expiration Date - Highest to Lowest",
      SDL_Point{0, 0}, "high to low", LogFiles::itemList);
  sortByExpirationHighToLow->initialize();
  this->mediator->addButton(sortByExpirationHighToLow);

  sortBy->addOption(std::move(sortByExpirationLowToHigh));
  sortBy->update();
  sortBy->addOption(std::move(sortByExpirationHighToLow));
  this->rootElement->addElement(std::move(sortBy));

  this->logger.log("Item list state constructed");
}

/**
 * Perform the appropriate action depending on which keyboard key has been pressed.
 *
 * @param None
 * @return The state the display is in after checking if any keys have been pressed
 */
EngineState ItemList::checkKeystates() {
  const Uint8* keystates = SDL_GetKeyboardState(NULL);

  if (keystates[SDL_SCANCODE_ESCAPE]) {
    this->logger.log("Escape key pressed in item list");
    return EngineState::PAUSE_MENU;
  }

  return EngineState::ITEM_LIST;
}

void ItemList::render() const {
  SDL_SetRenderDrawColor(this->displayGlobal.renderer, 0, 0, 0, 255); // Black background
  SDL_RenderClear(this->displayGlobal.renderer);
  this->rootElement->render();
  SDL_RenderPresent(this->displayGlobal.renderer);
}
