#include <SDL2/SDL.h>
#include <assert.h>
#include <glog/logging.h>
#include <iostream>
#include <string>

#include "../display_global.h"
#include "../elements/text.h"
#include "scanning.h"

/**
 * @param displayGlobal Global variables
 */
Scanning::Scanning(struct DisplayGlobal displayGlobal) : logger("scanning_state.txt") {
  this->logger.log("Constructing scanning state");

  this->currentState         = EngineState::SCANNING;
  this->displayGlobal        = displayGlobal;
  SDL_Surface* windowSurface = SDL_GetWindowSurface(this->displayGlobal.window);

  SDL_Rect rootRectangle = {0, 0, 0, 0};
  rootRectangle.w        = windowSurface->w;
  rootRectangle.h        = windowSurface->h;
  this->rootElement      = std::make_unique<Container>(rootRectangle);

  const char* fontPath               = "../display/fonts/16020_FUTURAM.ttf";
  const char* progressMessageContent = "Scanning In Progress";
  SDL_Color progressMessageColor     = {0, 255, 0, 255}; // Green
  SDL_Rect progressMessageRectangle  = {0, 100, 0, 0};
  std::unique_ptr<Text> progressMessage =
      std::make_unique<Text>(this->displayGlobal, progressMessageRectangle, fontPath,
                             progressMessageContent, 24, progressMessageColor);

  progressMessage->setCenteredHorizontal();
  this->rootElement->addElement(std::move(progressMessage));

  this->logger.log("Constructed scanning state");
}

/**
 * Perform the appropriate action depending on which keyboard key has been pressed.
 *
 * @param None
 * @return The state the display is in after checking if any keys have been pressed
 */
EngineState Scanning::checkKeystates() {
  const Uint8* keystates = SDL_GetKeyboardState(NULL);

  /*
  if (keystates[SDL_SCANCODE_ESCAPE]) {
    this->logger.log("Escape key pressed in scanning state");
    return EngineState::PAUSE_MENU;
  }
  */

  return EngineState::SCANNING;
}

/**
 * Render all scanning elements.
 *
 * @param None
 * @return None
 */
void Scanning::render() const {
  SDL_SetRenderDrawColor(this->displayGlobal.renderer, 0, 0, 0, 255); // Black background
  SDL_RenderClear(this->displayGlobal.renderer);
  this->rootElement->render();
  SDL_RenderPresent(this->displayGlobal.renderer);
}
