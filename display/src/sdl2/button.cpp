#include <SDL2/SDL.h>
#include <iostream>
#include <memory>

#include "button.h"
#include "display_global.h"
#include "element.h"

/**
 * Set the properties of the button.
 *
 * @param displayGlobal Global display variables
 * @param rectangle Rectangle to render the button with
 * @param text The text to print in the middle of the button
 * @return None
 */
Button::Button(struct DisplayGlobal displayGlobal,
               SDL_Rect rectangle,
               const std::string& text) {
  this->displayGlobal = displayGlobal;

  this->rectangle       = rectangle;
  this->backgroundColor = {255, 0, 0, 255}; // Red
  this->hoveredColor    = {0, 255, 0, 255}; // Green
  this->defaultColor    = {255, 0, 0, 255}; // Red

  SDL_Color textColor = {255, 255, 0, 255}; // Yellow
  this->text =
      std::make_unique<Text>(this->displayGlobal, "../display/fonts/16020_FUTURAM.ttf",
                             text.c_str(), 24, textColor, rectangle);

  // Center the text within the button
  this->text->centerHorizontal(this->rectangle);
  this->text->centerVertical(this->rectangle);
}

/**
 * Check if the mouse is over the button.
 *
 * @param mouseXPosition X position of the mouse
 * @param mouseYPosition Y position of the mouse
 * @return Whether or not the mouse is over the button
 */
bool Button::checkHovered(int mouseXPosition, int mouseYPosition) {
  if (mouseXPosition < this->rectangle.x) { // Outside left edge of button
    return false;
  }
  if (mouseXPosition >
      this->rectangle.x + this->rectangle.w) { // Outside right edge of button
    return false;
  }
  if (mouseYPosition < this->rectangle.y) { // Outside top edge of button
    return false;
  }
  if (mouseYPosition >
      this->rectangle.y + this->rectangle.h) { // Outside bottom edge of button
    return false;
  }
  return true;
}

/**
 * Render the button
 *
 * Input:
 * - None
 * Output: None
 */
void Button::render() {
  if (this->text->checkCenterHorizontal(this->rectangle) == false) {
    this->text->centerHorizontal(this->rectangle);
  }

  if (this->text->checkCenterVertical(this->rectangle) == false) {
    this->text->centerVertical(this->rectangle);
  }

  // Change color if hovered
  int mouseXPosition, mouseYPosition;
  SDL_GetMouseState(&mouseXPosition, &mouseYPosition); // Get the position of the mouse
  if (checkHovered(mouseXPosition, mouseYPosition)) {  // Mouse is hovered over the button
    this->backgroundColor = this->hoveredColor;        // Change to hovered color
  }
  else {                                        // Mouse is not hovered over the button
    this->backgroundColor = this->defaultColor; // Change to default color
  }

  // Set draw color and fill the button
  SDL_SetRenderDrawColor(this->displayGlobal.renderer, backgroundColor.r,
                         backgroundColor.g, backgroundColor.b, backgroundColor.a);
  SDL_RenderFillRect(this->displayGlobal.renderer, &this->rectangle);

  this->text->render();
}
