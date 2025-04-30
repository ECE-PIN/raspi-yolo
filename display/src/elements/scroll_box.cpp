#define SCROLL_AMOUNT 4

#include <SDL2/SDL.h>
#include <glog/logging.h>
#include <iostream>
#include <memory>
#include <vector>

#include "../../../food_item.h"
#include "../display_global.h"
#include "../sdl_debug.h"
#include "../sql_food.h"
#include "panel.h"
#include "scroll_box.h"

/**
 * @param displayGlobal
 * @param boundaryRectangle Rectangle defining offset within parent (if any) and width +
 * height
 * @param logFile The logfile to write logs to
 */
ScrollBox::ScrollBox(const struct DisplayGlobal& displayGlobal,
                     const std::string& logFile,
                     const SDL_Rect boundaryRectangle)
    : CompositeElement(displayGlobal, logFile, boundaryRectangle) {}

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

void ScrollBox::renderSelf() const {
  if (this->hasBorder) {
    renderBorder();
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
  switch (event.type) {
  case SDL_MOUSEBUTTONDOWN:
    handleMouseDown();
    break;

  case SDL_MOUSEMOTION:
    handleMouseMotion(event);
    break;

  case SDL_MOUSEBUTTONUP:
    handleMouseUp();
    break;

  default:
    break;
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

/**
 * Ensure that all panels have data consistent with what is in the database. Destroys all
 * panels and makes new ones.
 *
 * @param None
 * @return None
 */
void ScrollBox::refreshPanels() {
  std::vector<FoodItem> allFoodItems = readAllFoodItemsSorted(this->sortMethod);

  int longestName = 0;
  for (auto& foodItem : allFoodItems) {
    std::string name = foodItem.getName();
    int nameLength   = name.length();

    if (nameLength > longestName) {
      longestName = nameLength;
    }
  }

  this->children.clear();
  SDL_Rect boundaryRectangle = {0, topPanelPosition, 0, this->panelHeight};

  for (auto& foodItem : allFoodItems) {
    std::shared_ptr<Panel> newPanel =
        std::make_shared<Panel>(this->displayGlobal, this->logFile, boundaryRectangle,
                                foodItem.getId(), longestName);
    boundaryRectangle.y += panelHeight;

    newPanel->addFoodItem(foodItem, SDL_Point{0, 0});
    int borderThickness = 1;
    newPanel->addBorder(borderThickness);
    addElement(std::move(newPanel));
  }
}

/**
 * Move all panels up or down by the scroll amount. Zero out scroll amount so that it does
 * not grow unbounded.
 *
 * @param None
 * @return None
 */
void ScrollBox::scroll() {
  topPanelPosition += this->scrollAmount;
  for (auto& currPanel : this->children) {
    SDL_Point currPanelRelativePosition = currPanel->getPositionRelativeToParent();
    currPanelRelativePosition.y += this->scrollAmount;
    currPanel->setPositionRelativeToParent(currPanelRelativePosition);
  }
  this->scrollAmount = 0;
}

/**
 * SDL_MOUSEBUTTONDOWN
 *
 * @param None
 * @return None
 */
void ScrollBox::handleMouseDown() {
  if (checkMouseHovered() == false) {
    return;
  }
  this->held = true;
}

/**
 * SDL_MOUSEMOTION
 *
 * @param The SDL_MOUSEMOTION event to handle
 * @return None
 */
void ScrollBox::handleMouseMotion(const SDL_Event& event) {
  if (this->held) {
    this->scrollAmount += event.motion.y - this->previousMotion.y;
    scroll();
    this->previousMotion.y = event.motion.y;
  }

  this->previousMotion.y = event.motion.y;
}

/**
 * SDL_MOUSEBUTTONUP
 *
 * @param None
 * @return None
 */
void ScrollBox::handleMouseUp() { this->held = false; }
