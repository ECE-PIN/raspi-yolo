#define SCROLL_AMOUNT 4

#include <SDL2/SDL.h>
#include <glog/logging.h>
#include <iostream>
#include <memory>
#include <vector>

#include "../../../../food_item.h"
#include "../../sql_food.h"
#include "../display_global.h"
#include "../sdl_debug.h"
#include "panel.h"
#include "scroll_box.h"

/**
 * @param displayGlobal
 * @param boundaryRectangle Rectangle defining offset within parent (if any) and width +
 * height
 * @param logFile The logfile to write logs to
 */
ScrollBox::ScrollBox(struct DisplayGlobal displayGlobal,
                     const SDL_Rect& boundaryRectangle,
                     const std::string& logFile) {
  this->displayGlobal     = displayGlobal;
  this->previousUpdate    = std::chrono::steady_clock::now();
  this->boundaryRectangle = boundaryRectangle;
  this->logFile           = logFile;
  this->logger            = std::make_unique<Logger>(logFile);
}

void ScrollBox::setPanelHeight(int panelHeight) { this->panelHeight = panelHeight; }

/**
 * Update all panels in the scroll box with food item information. Assumed that one panel
 * corresponds to one food item. Therefore there will be as many panels in the scroll box
 * as there are food items passed. Don't want to refresh panels constantly (will cause
 * program to crash), so only refresh on a set interval.
 *
 * @param None
 * @return None
 */
void ScrollBox::updateSelf() {
  if (parent) {
    hasParentUpdate();
  }

  // Get time since last update
  this->currentUpdate = std::chrono::steady_clock::now();
  std::chrono::seconds updateDifference;
  updateDifference = std::chrono::duration_cast<std::chrono::seconds>(
      this->currentUpdate - this->previousUpdate);

  // 3 or more seconds since last update
  if (updateDifference.count() > 3) {
    refreshPanels();
    this->previousUpdate = this->currentUpdate;
  }
}

/**
 * Check if the scrollbox is hovered. If it is hovered and the event is a mousewheel
 * event, scroll the panels up or down accordingly.
 *
 * @param event The SDL event that has occured
 * @return None
 */
void ScrollBox::handleEventSelf(const SDL_Event& event) {
  if (checkMouseHovered() == false) {
    return;
  }

  if (event.type == SDL_MOUSEWHEEL) {
    if (event.wheel.y > 0) {
      scrollUp();
    }
    else if (event.wheel.y < 0) {
      scrollDown();
    }
  }
}

void ScrollBox::renderSelf() const {}

/**
 * Ensure that all panels have data consistent with what is in the database. Destroys all
 * panels and makes new ones.
 *
 * @param None
 * @return None
 */
void ScrollBox::refreshPanels() {
  std::vector<FoodItem> allFoodItems = readAllFoodItemsSorted(this->sortMethod);

  this->children.clear();
  SDL_Rect boundaryRectangle = {0, topPanelPosition, 0, this->panelHeight};

  for (auto& foodItem : allFoodItems) {
    std::shared_ptr<Panel> newPanel = std::make_shared<Panel>(
        this->displayGlobal, boundaryRectangle, foodItem.getId(), this->logFile);
    boundaryRectangle.y += panelHeight;

    newPanel->addFoodItem(foodItem, SDL_Point{0, 0});
    int borderThickness = 1;
    newPanel->addBorder(borderThickness);
    addElement(std::move(newPanel));
  }
}

/**
 * Move all panels within the scroll box up. Scroll amount is hardcoded.
 *
 * @param None
 * @return None
 */
void ScrollBox::scrollUp() {
  topPanelPosition -= this->scrollAmount;
  for (auto& currPanel : this->children) {
    SDL_Point currPanelRelativePosition = currPanel->getPositionRelativeToParent();
    currPanelRelativePosition.y -= this->scrollAmount;
    currPanel->setPositionRelativeToParent(currPanelRelativePosition);
  }
}

/**
 * Move all panels within the scroll box down. Scroll amount is hardcoded.
 *
 * @param None
 * @return None
 */
void ScrollBox::scrollDown() {
  topPanelPosition += this->scrollAmount;
  for (auto& currPanel : this->children) {
    SDL_Point currPanelRelativePosition = currPanel->getPositionRelativeToParent();
    currPanelRelativePosition.y += this->scrollAmount;
    currPanel->setPositionRelativeToParent(currPanelRelativePosition);
  }
}

void ScrollBox::setSortMethod(SortMethod sortMethod) {
  switch (sortMethod) {
  case SortMethod::LOW_TO_HIGH:
    this->sortMethod = SortMethod::LOW_TO_HIGH;
    this->logger->log("Scrollbox sort method changed to low to high");
    break;
  case SortMethod::HIGH_TO_LOW:
    this->sortMethod = SortMethod::HIGH_TO_LOW;
    this->logger->log("Scrollbox sort method changed to high to low");
    break;
  default:
    LOG(FATAL) << "Attempt to set sort method to invalid method";
  }

  refreshPanels();
}
