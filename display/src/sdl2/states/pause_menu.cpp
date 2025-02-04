#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <string>

#include "../display_global.h"
#include "pause_menu.h"

/**
 * Input:
 * - displayGlobal - Global display variables
 * Output: None
 */
PauseMenu::PauseMenu(struct DisplayGlobal displayGlobal) {
  this->displayGlobal        = displayGlobal;
  SDL_Surface* windowSurface = SDL_GetWindowSurface(this->displayGlobal.window);

  // Title
  const char* titleContent = "Paused";
  SDL_Color titleColor     = {0, 255, 0, 255}; // Green

  // x,y -> 100, 100. 0 width/height
  SDL_Rect titleRectangle = {
      100,
      100,
      0,
      0,
  };
  this->title = std::make_unique<Text>(this->displayGlobal, displayGlobal.futuramFontPath,
                                       titleContent, 24, titleColor, titleRectangle);
  this->title->centerHorizontal(windowSurface);

  // Resume button
  SDL_Rect resumeButtonRectangle = {200, 150, 200, 50};
  this->resumeButton =
      std::make_unique<Button>(this->displayGlobal, resumeButtonRectangle, "Resume");
  this->resumeButton->centerHorizontal(windowSurface);

  // Main menu button
  SDL_Rect mainMenuButtonRectangle = {200, 225, 200, 50};
  this->mainMenuButton =
      std::make_unique<Button>(this->displayGlobal, mainMenuButtonRectangle, "Main Menu");
  this->mainMenuButton->centerHorizontal(windowSurface);
}

/**
 * Handle SDL events that occur in the pause menu state.
 *
 * Input:
 * - displayIsRunning - Whether or not the display is running.
 * Output: Current state the display is in.
 */
int PauseMenu::handleEvents(bool* displayIsRunning) {
  SDL_Event event;
  int returnValue = 2;
  while (SDL_PollEvent(&event) != 0) { // While there are events in the queue
    switch (event.type) {              // Check which type of event
    case SDL_QUIT:
      *displayIsRunning = false;
      break;

    case SDL_MOUSEBUTTONDOWN:
      if (this->resumeButton->checkHovered(event.motion.x, event.motion.y) == 0) {
        ;
      }
      else {
        //        writeToLogFile(this->displayGlobal.logFile,
        //                      "Resume was pressed, switching from pause menu to
        //                      displayplay");
        returnValue = 1; // Mouse was over button, switch to displayplay state
        break;
      }

      if (this->mainMenuButton->checkHovered(event.motion.x, event.motion.y) == 0) {
        ;
      }
      else {
        //      writeToLogFile(this->displayGlobal.logFile,
        //                     "Main menu was pressed, switching from pause menu to main
        //                     menu");
        returnValue = 0; // Mouse was over button, switch to main menu state
        break;
      }

      break;

    default:
      break;
    }
  }
  return returnValue;
}

/**
 * Render paused and the button
 *
 * Input: None
 * Output: None
 */
void PauseMenu::render() {
  SDL_SetRenderDrawColor(this->displayGlobal.renderer, 0, 0, 0, 255); // Black background
  SDL_RenderClear(this->displayGlobal.renderer);
  this->resumeButton->render();
  this->mainMenuButton->render();
  this->title->render();
  SDL_RenderPresent(this->displayGlobal.renderer);
}
