#include <SDL2/SDL.h>
#include <iostream>

#include "element.h"

void Element::render() const {
  if (this->hasBorder) {
    renderBorder();
  }
}

/**
 * Ensure that the positioning of the element is correct. Make sure that if need be, it is
 * centered with respect to its parent (if it has one) and that it is in the correct
 * position relative to its parent (if it has one).
 *
 * @param None
 * @return None
 */
void Element::update() {
  if (parent) {
    hasParentUpdate();
  }
}

/**
 * Updates to perform if the element has a parent. Checks centering within parent and
 * ensures the position of the element is correct giving its current position relative to
 * parent.
 *
 * @param None
 * @return None
 */
void Element::hasParentUpdate() {
  if (this->centerWithinParent) {
    if (checkCenterVertical() == false) {
      centerVertical();
    }
    if (checkCenterHorizontal() == false) {
      centerHorizontal();
    }
  }
  else if (this->centerVerticalWithinParent) {
    if (checkCenterVertical() == false) {
      centerVertical();
    }
  }
  else if (this->centerHorizontalWithinParent) {
    if (checkCenterHorizontal() == false) {
      centerHorizontal();
    }
  }
  updatePosition();
}

void Element::updatePosition() {
  SDL_Rect parentBoundaryRectangle = parent->getBoundaryRectangle();
  this->boundaryRectangle.x =
      parentBoundaryRectangle.x + this->positionRelativeToParent.x;
  this->boundaryRectangle.y =
      parentBoundaryRectangle.y + this->positionRelativeToParent.y;
}

std::string Element::getContent() const { return "no content"; }

void Element::setMediator(std::shared_ptr<Mediator> mediator) {
  this->mediator = mediator;
}
void Element::setParent(Element* parent) { this->parent = parent; }

void Element::setPositionRelativeToParent(const SDL_Point& relativePostion) {
  this->positionRelativeToParent = relativePostion;
}
SDL_Point Element::getPositionRelativeToParent() {
  return this->positionRelativeToParent;
}

SDL_Rect Element::getBoundaryRectangle() { return this->boundaryRectangle; }
void Element::setBoundaryRectangle(SDL_Rect boundaryRectangle) {
  this->boundaryRectangle = boundaryRectangle;
};

void Element::setCentered() { this->centerWithinParent = true; }

/**
 * Centering vertically means centering on the y axis.
 */
void Element::setCenteredVertical() { this->centerVerticalWithinParent = true; }
bool Element::checkCenterVertical() {
  bool centered = false;
  if (this->positionRelativeToParent.y ==
      (this->parent->boundaryRectangle.h / 2 - this->boundaryRectangle.h / 2)) {
    centered = true;
  }
  return centered;
}
void Element::centerVertical() {
  this->positionRelativeToParent.y =
      (this->parent->boundaryRectangle.h / 2 - this->boundaryRectangle.h / 2);
}

/**
 * Centering horizontally means centering on the x axis.
 */
void Element::setCenteredHorizontal() { this->centerHorizontalWithinParent = true; }
bool Element::checkCenterHorizontal() {
  bool centered = false;
  if (this->positionRelativeToParent.x ==
      (this->parent->boundaryRectangle.w / 2 - this->boundaryRectangle.w / 2)) {
    centered = true;
  }
  return centered;
}
void Element::centerHorizontal() {
  this->positionRelativeToParent.x =
      ((this->parent->boundaryRectangle.w - this->boundaryRectangle.w) / 2);
}

/**
 * Check if the mouse is over the element.
 *
 * @param None
 * @return Whether or not the mouse is over the button
 */
bool Element::checkHovered() {
  int mouseXPosition, mouseYPosition;
  SDL_GetMouseState(&mouseXPosition, &mouseYPosition);

  // Outside left edge
  if (mouseXPosition < this->boundaryRectangle.x) {
    return false;
  }

  // Outside right edge
  if (mouseXPosition > this->boundaryRectangle.x + this->boundaryRectangle.w) {
    return false;
  }

  // Outside top edge
  if (mouseYPosition < this->boundaryRectangle.y) {
    return false;
  }

  // Outside bottom edge
  if (mouseYPosition > this->boundaryRectangle.y + this->boundaryRectangle.h) {
    return false;
  }

  return true;
}

/**
 * Add a border to an element.
 *
 * @param borderThickness How many pixels thick the border should be
 * @return None
 */
void Element::addBorder(const int& borderThickness) {
  this->hasBorder       = true;
  this->borderThickness = borderThickness;
}

/**
 * Render a border around an object. Draws 4 lines on each side of the element's
 * boundaryRectangle.
 *
 * @param None
 * @return None
 */
void Element::renderBorder() const {
  SDL_SetRenderDrawColor(this->displayGlobal.renderer, 255, 255, 255, 255);
  // Draw the top border
  for (int i = 0; i < borderThickness; i++) {
    SDL_RenderDrawLine(this->displayGlobal.renderer,
                       this->positionRelativeToParent.x - i, // Start X
                       this->positionRelativeToParent.y - i, // Start Y
                       this->positionRelativeToParent.x + this->boundaryRectangle.w +
                           i,                                 // End X
                       this->positionRelativeToParent.y - i); // End Y
  }

  // Draw the bottom border
  for (int i = 0; i < borderThickness; i++) {
    SDL_RenderDrawLine(
        this->displayGlobal.renderer,
        this->positionRelativeToParent.x - i,                              // Start X
        this->positionRelativeToParent.y + this->boundaryRectangle.h + i,  // Start Y
        this->positionRelativeToParent.x + this->boundaryRectangle.w + i,  // End X
        this->positionRelativeToParent.y + this->boundaryRectangle.h + i); // End Y
  }

  // Draw the left border
  for (int i = 0; i < borderThickness; i++) {
    SDL_RenderDrawLine(this->displayGlobal.renderer,
                       this->positionRelativeToParent.x - i, // Start X
                       this->positionRelativeToParent.y - i, // Start Y
                       this->positionRelativeToParent.x - i, // End X
                       this->positionRelativeToParent.y + this->boundaryRectangle.h +
                           i); // End Y
  }

  // Draw the right border
  for (int i = 0; i < borderThickness; i++) {
    SDL_RenderDrawLine(
        this->displayGlobal.renderer,
        this->positionRelativeToParent.x + this->boundaryRectangle.w + i,  // Start X
        this->positionRelativeToParent.y - i,                              // Start Y
        this->positionRelativeToParent.x + this->boundaryRectangle.w + i,  // End X
        this->positionRelativeToParent.y + this->boundaryRectangle.h + i); // End Y
  }
}

void Element::setupPosition(const SDL_Rect& boundaryRectangle) {
  this->boundaryRectangle          = boundaryRectangle;
  this->positionRelativeToParent.x = boundaryRectangle.x;
  this->positionRelativeToParent.y = boundaryRectangle.y;

  if (parent) {
    SDL_Rect parentRectangle  = this->parent->getBoundaryRectangle();
    this->boundaryRectangle.x = parentRectangle.x + this->positionRelativeToParent.x;
    this->boundaryRectangle.y = parentRectangle.y + this->positionRelativeToParent.y;
  }
}
