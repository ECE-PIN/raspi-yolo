#include <SDL2/SDL.h>
#include <iostream>
#include <memory>

#include "../display_global.h"
#include "button.h"
#include "element.h"

/**
 * @param displayGlobal Global display variables
 * @param boundaryRectangle Rectangle to describe the position relative to parent element
 * as well as the width and height. The x and y parameters of the rectangle are used as
 * the position relative to the parent.
 * @param textContent The text to print in the middle of the button
 * @param textPadding How offset the text should be from parent
 * @param callback The callback function to execute when the button is clicked
 */
Button::Button(struct DisplayGlobal displayGlobal,
               const SDL_Rect& boundaryRectangle,
               const std::string& textContent,
               const SDL_Point& textPadding,
               std::function<void()> callback,
               const std::string& logFile)
    : textContent(textContent), textPadding(textPadding), onClick(callback) {
  this->logger = std::make_unique<Logger>(logFile);

  this->displayGlobal = displayGlobal;

  setupPosition(boundaryRectangle);

  // Colors
  this->backgroundColor = {255, 0, 0, 255}; // Red
  this->hoveredColor    = {0, 255, 0, 255}; // Green
  this->defaultColor    = {255, 0, 0, 255}; // Red

  // Button Text
  SDL_Color textColor        = {255, 255, 0, 255}; // Yellow
  SDL_Rect textRect          = {textPadding.x, textPadding.y, 0, 0};
  std::unique_ptr<Text> text = std::make_unique<Text>(
      this->displayGlobal, textRect, "../display/fonts/16020_FUTURAM.ttf",
      this->textContent.c_str(), 24, textColor);
  text->setCentered();

  // Size based on text
  if (this->boundaryRectangle.w == 0 && this->boundaryRectangle.h == 0) {
    SDL_Rect textboundaryRectangle = text->getBoundaryRectangle();
    this->boundaryRectangle.w      = textboundaryRectangle.w + textPadding.x;
    this->boundaryRectangle.h      = textboundaryRectangle.h + textPadding.y;
  }

  addElement(std::move(text));
}

Button::Button(struct DisplayGlobal displayGlobal,
               const SDL_Rect& boundaryRectangle,
               const std::string& textContent,
               const SDL_Point& textPadding,
               const std::string& notifyMessage,
               const std::string& logFile)
    : textContent(textContent), textPadding(textPadding), notifyMessage(notifyMessage) {
  this->logger = std::make_unique<Logger>(logFile);
  this->logger->log("Constructing " + this->textContent + " button");

  this->displayGlobal = displayGlobal;

  setupPosition(boundaryRectangle);

  // Colors
  this->backgroundColor = {255, 0, 0, 255}; // Red
  this->hoveredColor    = {0, 255, 0, 255}; // Green
  this->defaultColor    = {255, 0, 0, 255}; // Red

  // Button Text
  SDL_Color textColor        = {255, 255, 0, 255}; // Yellow
  SDL_Rect textRect          = {textPadding.x, textPadding.y, 0, 0};
  std::unique_ptr<Text> text = std::make_unique<Text>(
      this->displayGlobal, textRect, "../display/fonts/16020_FUTURAM.ttf",
      this->textContent.c_str(), 24, textColor);
  text->setCentered();

  // Size based on text
  if (this->boundaryRectangle.w == 0 && this->boundaryRectangle.h == 0) {
    SDL_Rect textboundaryRectangle = text->getBoundaryRectangle();
    this->boundaryRectangle.w      = textboundaryRectangle.w + textPadding.x;
    this->boundaryRectangle.h      = textboundaryRectangle.h + textPadding.y;
  }

  addElement(std::move(text));
  this->logger->log(this->textContent + " button constructed");
}

void Button::initialize() {
  this->logger->log("Initializing " + this->textContent + " button");

  this->onClick = [this]() {
    if (auto mediatorShared = this->mediator.lock()) {
      this->logger->log(this->textContent + " button notifying mediator with message " +
                        this->notifyMessage);

      mediatorShared->notify(shared_from_this(), this->notifyMessage);

      this->logger->log(this->textContent + " button successfully notified mediator");
    }
    else {
      this->logger->log(this->textContent + " button unable to get mediator lock");
      // uh oh
    }
  };

  this->logger->log("Successfully initialized " + this->textContent + " button");
}

/**
 * Change color if the cursor is hovered over the button.
 *
 * @param None
 * @return None
 */
void Button::updateSelf() {
  if (parent) {
    hasParentUpdate();
  }

  // Change color if hovered
  if (checkHovered()) {
    this->logger->log(this->textContent + " button hovered");
    this->backgroundColor = this->hoveredColor;
  }
  else {
    this->backgroundColor = this->defaultColor;
  }
}

/**
 * Render the background color
 *
 * @param None
 * @return None
 */
void Button::renderSelf() const {
  // Set draw color and fill the button
  SDL_SetRenderDrawColor(this->displayGlobal.renderer, backgroundColor.r,
                         backgroundColor.g, backgroundColor.b, backgroundColor.a);
  SDL_RenderFillRect(this->displayGlobal.renderer, &this->boundaryRectangle);
}

/**
 * If the user is hovered over the button when a click event occurs, call the callback
 * function.
 *
 * @param event An SDL event that has occured.
 * @return None
 */
void Button::handleEventSelf(const SDL_Event& event) {
  if (event.type == SDL_MOUSEBUTTONDOWN) {
    if (checkHovered() == true) {
      this->logger->log(this->textContent + " button clicked");
      onClick();
      this->logger->log(this->textContent + " button click callback successful");
    }
  }
}
