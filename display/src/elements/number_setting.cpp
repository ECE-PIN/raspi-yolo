#include <glog/logging.h>
#include <iostream>
#include <sqlite3.h>

#include "../../../food_item.h"
#include "../sdl_debug.h"
#include "../sql_food.h"
#include "number_setting.h"

/**
 * Get the current value of the setting from the database and set the content to it.
 *
 * @param displayGlobal
 * @param boundaryRectangle Rectangle defining offset within parent (if any) and width +
 * height
 * @param settingId The primary key of the food item corresponding to this object
 */
NumberSetting::NumberSetting(const struct DisplayGlobal& displayGlobal,
                             const std::string& logFile,
                             const SDL_Rect boundaryRectangle,
                             const int settingId)
    : CompositeElement(displayGlobal, logFile, boundaryRectangle), settingId(settingId) {
  std::shared_ptr<Button> decreaseButton =
      std::make_shared<Button>(this->displayGlobal, this->logFile, SDL_Rect{0, 0, 0, 0},
                               "-", SDL_Point{0, 0}, [this]() { this->settingValue--; });
  addElement(std::move(decreaseButton));

  std::shared_ptr<Text> text = std::make_shared<Text>(
      this->displayGlobal, this->logFile, SDL_Rect{0, 0, 0, 0},
      DisplayGlobal::futuramFontPath, "0", 24, SDL_Color{0, 255, 0, 255});
  addElement(std::move(text));

  std::shared_ptr<Button> increaseButton =
      std::make_shared<Button>(this->displayGlobal, this->logFile, SDL_Rect{0, 0, 0, 0},
                               "+", SDL_Point{0, 0}, [this]() { this->settingValue++; });
  addElement(std::move(increaseButton));

  if (settingId != -1) {
    FoodItem foodItem  = readFoodItemById(this->settingId);
    this->settingValue = foodItem.getQuantity();
    this->children[1]->setContent(std::to_string(this->settingValue));
  }
  else {
    this->settingValue = 0;
  }
}

/**
 * Position all children so that they are right next to each other. Children will be
 * ordered from left to right corresponding to lower to higher indicies in the children
 * vector. In other words, children[0] will be positioned all the way to the left and the
 * following elements will be positioned to its right. Also ensure that the value of the
 * setting between the buttons corresponds to what is stored in the database.
 *
 * @param None
 * @return None
 */
void NumberSetting::updateSelf() {
  if (parent) {
    hasParentUpdate();
  }

  this->boundaryRectangle.w = 0;
  // TODO change to range based for loop
  for (int i = 0; i < this->children.size(); i++) {
    SDL_Point childRelativePosition = this->children[i]->getPositionRelativeToParent();
    childRelativePosition.y         = this->positionRelativeToParent.y;
    if (i == 0) {
      childRelativePosition.x = 0;
    }
    else {
      SDL_Point leftRelativePosition =
          this->children[i - 1]->getPositionRelativeToParent();
      SDL_Rect leftBoundaryRectangle = this->children[i - 1]->getBoundaryRectangle();
      this->boundaryRectangle.w += leftBoundaryRectangle.w;
      childRelativePosition.x = leftRelativePosition.x + leftBoundaryRectangle.w;
    }
    this->children[i]->setPositionRelativeToParent(childRelativePosition);
  }

  if (this->settingId != -1) {
    FoodItem foodItem    = readFoodItemById(this->settingId);
    int foodItemQuantity = foodItem.getQuantity();
    if (foodItemQuantity == 0) {
      deleteById(this->settingId);
    }
    else if (foodItemQuantity != this->settingValue) {
      updateFoodItemQuantity(this->settingId, this->settingValue);
    }
    /*
    else {
      this->children[1]->setContent(std::to_string(this->settingValue));
    }
  */
  }
  this->children[1]->setContent(std::to_string(this->settingValue));
}

void NumberSetting::handleEventSelf(const SDL_Event& event) {}
