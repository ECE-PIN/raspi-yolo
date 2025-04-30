#ifndef PANEL_H
#define PANEL_H

#include <SDL2/SDL.h>
#include <memory>
#include <vector>

#include "../../../food_item.h"
#include "../display_global.h"
#include "button.h"
#include "element.h"
#include "number_setting.h"
#include "text.h"

/**
 * At the moment this is an element specially designed for use within the scrollbox. It is
 * also specific to displaying information about a food item.
 *
 * @see ScrollBox
 */
class Panel : public CompositeElement {
public:
  Panel(const struct DisplayGlobal& displayGlobal,
        const std::string& logFile,
        const SDL_Rect boundaryRectangle,
        const int id);

  void setId(const int id);

  void addText(const std::string& fontPath,
               const std::string& content,
               const int& fontSize,
               const SDL_Color& color,
               const SDL_Point& relativePosition);
  void addFoodItem(const FoodItem& foodItem, const SDL_Point& relativePosition);

  void updateSelf() override;
  void handleEventSelf(const SDL_Event& event) override;

private:
  int id                         = -1;
  const std::string WEIGHT_UNITS = "lbs";

  void addFoodItemName(const FoodItem& foodItem, const SDL_Point& relativePosition);
  void addFoodItemExpirationDate(const FoodItem& foodItem,
                                 const SDL_Point& relativePosition);
  void addFoodItemWeight(const FoodItem& foodItem, const SDL_Point& relativePosition);
};

#endif
