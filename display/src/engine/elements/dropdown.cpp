#include "composite_element.h"
#include "dropdown.h"

/**
 * @param displayGlobal Global display variables
 * @param boundaryRectangle Rectangle defining offset from parent and width + height
 * @param titleContent What the title of the drop down should be
 */
Dropdown::Dropdown(struct DisplayGlobal displayGlobal,
                   const SDL_Rect& boundaryRectangle,
                   const std::string& titleContent) {
  this->displayGlobal = displayGlobal;

  setupPosition(boundaryRectangle);

  std::unique_ptr<Text> title = std::make_unique<Text>(
      this->displayGlobal, SDL_Rect{0, 0, 0, 0}, this->displayGlobal.futuramFontPath,
      titleContent, 24, SDL_Color{0, 255, 0, 255});
  addElement(std::move(title));
}

/**
 * Add an option to the drop down menu. Only supports adding options as buttons. The new
 * option will be added below the lowest current option in the menu.
 *
 * @param newOption The new option button to add to the dropdown.
 * @return None
 */
void Dropdown::addOption(std::shared_ptr<Button> newOption) {
  SDL_Point newOptionRelativePosition = newOption->getPositionRelativeToParent();
  SDL_Rect lowestOptionRectangle      = this->children.back()->getBoundaryRectangle();
  newOptionRelativePosition.y         = lowestOptionRectangle.h + lowestOptionRectangle.y;
  newOption->setPositionRelativeToParent(newOptionRelativePosition);
  addElement(std::move(newOption));
}

void Dropdown::handleEventSelf(const SDL_Event& event) {}
