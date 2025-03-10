#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <string>

#include "../../log_files.h"
#include "../display_global.h"
#include "pause_menu.h"

/**
 * Input:
 * @param displayGlobal Global display variables
 * @return None
 */
PauseMenu::PauseMenu(struct DisplayGlobal displayGlobal) : logger(LogFiles::pauseMenu) {
  this->logger.log("Constructing pause menu state");

  this->currentState         = EngineState::PAUSE_MENU;
  this->displayGlobal        = displayGlobal;
  SDL_Surface* windowSurface = SDL_GetWindowSurface(this->displayGlobal.window);

  SDL_Rect rootRectangle = {0, 0, 0, 0};
  rootRectangle.w        = windowSurface->w;
  rootRectangle.h        = windowSurface->h;
  this->rootElement      = std::make_unique<Container>(rootRectangle);

  // Title text
  const char* titleContent = "Paused";
  SDL_Color titleColor     = {0, 255, 0, 255}; // Green
  SDL_Rect titleRectangle  = {0, 100, 0, 0};
  std::unique_ptr<Text> title =
      std::make_unique<Text>(this->displayGlobal, titleRectangle,
                             displayGlobal.futuramFontPath, titleContent, 24, titleColor);
  title->setCenteredHorizontal();
  this->rootElement->addElement(std::move(title));

  // Resume button
  SDL_Rect resumeButtonRectangle       = {200, 150, 200, 50};
  std::unique_ptr<Button> resumeButton = std::make_unique<Button>(
      this->displayGlobal, resumeButtonRectangle, "Resume", SDL_Point{10, 10},
      [this]() { this->currentState = EngineState::SCANNING; }, LogFiles::pauseMenu);
  resumeButton->setCenteredHorizontal();
  this->rootElement->addElement(std::move(resumeButton));

  // Main menu button
  SDL_Rect mainMenuButtonRectangle       = {200, 225, 200, 50};
  std::unique_ptr<Button> mainMenuButton = std::make_unique<Button>(
      this->displayGlobal, mainMenuButtonRectangle, "Main Menu", SDL_Point{10, 10},
      [this]() { this->currentState = EngineState::MAIN_MENU; }, LogFiles::pauseMenu);
  mainMenuButton->setCenteredHorizontal();
  this->rootElement->addElement(std::move(mainMenuButton));

  this->logger.log("Constructed pause menu state");
}

/**
 * Render all elements.
 *
 * @param None
 * @return None
 */
void PauseMenu::render() const {
  SDL_SetRenderDrawColor(this->displayGlobal.renderer, 0, 0, 0, 255); // Black background
  SDL_RenderClear(this->displayGlobal.renderer);
  this->rootElement->render();
  SDL_RenderPresent(this->displayGlobal.renderer);
}
