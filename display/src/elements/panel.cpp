#include <SDL2/SDL.h>
#include <iostream>
#include <memory>
#include <vector>

#include "../display_global.h"
#include "../sdl_debug.h"
#include "../sql_food.h"
#include "button.h"
#include "element.h"
#include "panel.h"
#include "text.h"

/**
 * @param displayGlobal
 * @param boundaryRectangle Rectangle defining offset within parent (if any) and width +
 * height
 * @param id The primary key of the food item corresponding to this panel
 * @param logFile Logfile to write logs to
 */
Panel::Panel(const struct DisplayGlobal& displayGlobal,
             const std::string& logFile,
             const SDL_Rect boundaryRectangle,
             const int id)
    : CompositeElement(displayGlobal, logFile, boundaryRectangle), id(id) {}

/**
 * Set a new id. Updates information within the panel according to the new id.
 *
 * @param id The new id
 * @return None
 */
void Panel::setId(const int id) {
  this->id = id;
  removeAllChildren();
  FoodItem foodItem = readFoodItemById(this->id);
  addFoodItem(foodItem, SDL_Point{0, 0});
}

/**
 * Add some text to a panel. Whatever text is added first will be displayed on the left
 * and any text added after that will be displayed moving right.
 *
 * @param fontPath File path to the font of the text
 * @param content The actual string of text to add
 * @param fontSize How big the text font should be
 * @param color RGB value for the text color
 * @param relativePosition The position of the text in relation to its parent.
 * @return None
 */
void Panel::addText(const std::string& fontPath,
                    const std::string& content,
                    const int& fontSize,
                    const SDL_Color& color,
                    const SDL_Point& relativePosition) {
  SDL_Rect textRectangle = {relativePosition.x, relativePosition.y, 0, 0};
  std::shared_ptr<Text> text =
      std::make_shared<Text>(this->displayGlobal, this->logFile, textRectangle, fontPath,
                             content, fontSize, color);
  addElement(std::move(text));
}

/**
 * Adds relevent food item information.
 *
 * @param foodItem The food item to add to the panel
 * @param relativePosition The position of the Element to be added relative to its parent
 * @return None
 */
void Panel::addFoodItem(const FoodItem& foodItem, const SDL_Point& relativePosition) {
  addFoodItemName(foodItem, relativePosition);
  addFoodItemExpirationDate(foodItem, relativePosition);

  std::shared_ptr<NumberSetting> itemQuantity = std::make_shared<NumberSetting>(
      this->displayGlobal, this->logFile, SDL_Rect{0, 0, 0, 0}, this->id);
  addElement(std::move(itemQuantity));

  addFoodItemWeight(foodItem, relativePosition);
}

/**
 * Ensures that all of the elements within the panel are aligned as desired.
 *
 * @param None
 * @return None
 */
void Panel::updateSelf() {
  if (parent) {
    hasParentUpdate();
  }

  // Align all children right next to each other
  for (int i = 0; i < this->children.size(); i++) {
    SDL_Point childRelativePosition = this->children[i]->getPositionRelativeToParent();
    if (i == 0) {
      childRelativePosition.x = 0;
    }
    else {
      SDL_Point leftRelativePosition =
          this->children[i - 1]->getPositionRelativeToParent();
      SDL_Rect leftBoundaryRectangle = this->children[i - 1]->getBoundaryRectangle();
      childRelativePosition.x        = leftRelativePosition.x + leftBoundaryRectangle.w;
    }
    this->children[i]->setPositionRelativeToParent(childRelativePosition);
  }
}

void Panel::handleEventSelf(const SDL_Event& event) {}

/**
 * Add the name of a food item to a panel.
 *
 * @param foodItem The food item to take the name from
 * @param relativePosition Position of the food item name relative to its parent
 * @return None
 */
void Panel::addFoodItemName(const FoodItem& foodItem, const SDL_Point& relativePosition) {
  std::string fontPath = this->displayGlobal.futuramFontPath;
  int fontSize         = 24;
  SDL_Color textColor  = {0, 255, 0, 255}; // Green

  addText(fontPath, foodItem.getName(), fontSize, textColor, relativePosition);
}

/**
 * Add the expiration date of a food item to a panel. Expires: mm/dd/yyyy.
 *
 * @param foodItem The food item to take the expiration date from
 * @param relativePosition Position of the expiration date relative to its parent
 * @return None
 */
void Panel::addFoodItemExpirationDate(const FoodItem& foodItem,
                                      const SDL_Point& relativePosition) {
  std::string fontPath                       = this->displayGlobal.futuramFontPath;
  int fontSize                               = 24;
  SDL_Color textColor                        = {0, 255, 0, 255}; // Green
  std::chrono::year_month_day expirationDate = foodItem.getExpirationDate();

  // Expires
  addText(fontPath, " Expires: ", fontSize, textColor, relativePosition);

  // Month
  std::string expirationDateMonth =
      std::to_string(static_cast<unsigned>(expirationDate.month()));
  addText(fontPath, expirationDateMonth, fontSize, textColor, relativePosition);

  // Slash
  addText(fontPath, "/", fontSize, textColor, relativePosition);

  // Date
  std::string expirationDateDay =
      std::to_string(static_cast<unsigned>(expirationDate.day()));
  addText(fontPath, expirationDateDay, fontSize, textColor, relativePosition);

  // Slash
  addText(fontPath, "/", fontSize, textColor, relativePosition);

  // Year
  std::string expirationDateYear =
      std::to_string(static_cast<int>(expirationDate.year()));
  addText(fontPath, expirationDateYear, fontSize, textColor, relativePosition);
}

void Panel::addFoodItemWeight(const FoodItem& foodItem,
                              const SDL_Point& relativePosition) {
  std::string fontPath = this->displayGlobal.futuramFontPath;
  int fontSize         = 24;
  SDL_Color textColor  = {0, 255, 0, 255}; // Green

  addText(fontPath, " Wt: ", fontSize, textColor, relativePosition);

  std::string weight = std::to_string(foodItem.getWeight());
  addText(fontPath, weight, fontSize, textColor, relativePosition);

  addText(fontPath, this->WEIGHT_UNITS, fontSize, textColor, relativePosition);
}
