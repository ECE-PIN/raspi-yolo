#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>
#include <glog/logging.h>
#include <iostream>

#include "../../endpoints.h"

#include "display_engine.h"
#include "display_global.h"
#include "states/state.h"

/**
 * @param windowTitle Title of the display window
 * @param windowXPosition X position of the display window
 * @param windowYPostition Y position of the display window
 * @param screenWidth Width of the display window in pixels
 * @param screenHeight Height of the display window in pixels
 * @param fullscreen Whether or not the display window should be fullscreen
 * @param context The zeroMQ context with which to create sockets with
 */
DisplayEngine::DisplayEngine(const char* windowTitle,
                             int windowXPosition,
                             int windowYPosition,
                             int screenWidth,
                             int screenHeight,
                             bool fullscreen,
                             const zmqpp::context& context)
    : logger("display_engine.txt") {
  this->logger.log("Constructing display engine");

  DisplayHandler::init(context);

  this->displayGlobal.window = setupWindow(windowTitle, windowXPosition, windowYPosition,
                                           screenWidth, screenHeight, fullscreen);

  initializeEngine(this->displayGlobal.window);

  // States
  this->scanning = std::make_unique<Scanning>(this->displayGlobal, EngineState::SCANNING);
  this->itemList =
      std::make_unique<ItemList>(this->displayGlobal, EngineState::ITEM_LIST);
  this->zeroWeight =
      std::make_unique<ZeroWeight>(this->displayGlobal, EngineState::ZERO_WEIGHT);
  this->cancelScanConfirmation = std::make_unique<CancelScanConfirmation>(
      this->displayGlobal, EngineState::CANCEL_SCAN_CONFIRMATION);
  this->scanSuccess =
      std::make_unique<ScanSuccess>(this->displayGlobal, EngineState::SCAN_SUCCESS);

  this->engineState = this->itemList.get();

  displayIsRunning = true;
  this->logger.log("Engine is constructed and now running");
}

void DisplayEngine::start() {
  LOG(INFO) << "SDL display process started successfully";

  std::chrono::milliseconds msPerFrame = std::chrono::milliseconds(16);

  while (this->displayIsRunning) {
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    handleEvents();
    handleStateChange();
    update();
    renderState();

    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    std::chrono::milliseconds sleepDuration =
        std::chrono::duration_cast<std::chrono::milliseconds>(start + msPerFrame - now);
    std::this_thread::sleep_for(sleepDuration);
  }

  clean();
}

/**
 * Setup the SDL display window.
 *
 * @param windowTitle The name of the window. Is what is displayed on the top bezel of
 * the window when the display is running.
 * @param windowXPosition The X position of the window on the user's screen.
 * @param windowYPosition The Y position of the window on the user's screen.
 * @param screenWidth The width of the screen in pixels.
 * @param screenHeight The height of the screen in pixels.
 * @param fullscreen Whether or not the display window should be fullscreen.
 * @return Pointer to the SDL display window.
 */
SDL_Window* DisplayEngine::setupWindow(const char* windowTitle,
                                       int windowXPosition,
                                       int windowYPosition,
                                       int screenWidth,
                                       int screenHeight,
                                       bool fullscreen) {
  this->logger.log("Creating SDL display window");

  int flags = 0;
  if (fullscreen) {
    flags = SDL_WINDOW_FULLSCREEN;
  }

  try {
    return SDL_CreateWindow(windowTitle, windowXPosition, windowYPosition, screenWidth,
                            screenHeight, flags);
  } catch (...) {
    LOG(FATAL) << "Error setting up SDL display window";
  }

  this->logger.log("SDL display window created");
}

/**
 * Setup SDL, the renderer, and TTF. Renderer is part of the global display objects.
 *
 * @param window The SDL display window
 * @return None
 */
void DisplayEngine::initializeEngine(SDL_Window* window) {
  this->logger.log("Initializing engine");
  try {
    int sdlInitReturn = SDL_Init(SDL_INIT_EVERYTHING);
    if (sdlInitReturn != 0) {
      throw;
    }
  } catch (...) {
    LOG(FATAL) << "Failed to initialize engine";
    exit(1);
  }

  // Create renderer
  try {
    this->displayGlobal.renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!this->displayGlobal.renderer) {
      throw;
    }
    SDL_SetRenderDrawColor(this->displayGlobal.renderer, 255, 255, 255, 255);
  } catch (...) {
    LOG(FATAL) << "Error creating renderer";
    exit(1);
  }

  // Initialize TTF
  try {
    int ttfInitReturn = TTF_Init();
    if (ttfInitReturn == -1) {
      throw;
    }
  } catch (...) {
    LOG(FATAL) << "Failed to initialize TTF";
    exit(1);
  }
  this->logger.log("Engine initialized");
}

void DisplayEngine::handleStateChange() {
  if (this->engineState->checkStateChange()) {
    this->engineState->exit();
    EngineState currentState = this->engineState->getCurrentState();

    switch (currentState) {
    case EngineState::SCANNING:
      this->engineState = this->scanning.get();
      break;

    case EngineState::ITEM_LIST:
      this->engineState = this->itemList.get();
      break;

    case EngineState::ZERO_WEIGHT:
      this->engineState = this->zeroWeight.get();
      break;

    case EngineState::CANCEL_SCAN_CONFIRMATION:
      this->engineState = this->cancelScanConfirmation.get();
      break;

    case EngineState::SCAN_SUCCESS:
      this->engineState = this->scanSuccess.get();
      break;

    default:
      break;
    }
    this->engineState->enter();
  }
}

/**
 * Check the current state, and call that state's handle events method.
 *
 * @param engineToDisplay Pipe from the display engine to the display
 * @param displayToEngine Pipe from the display to the displayEngine
 * @return None
 */
void DisplayEngine::handleEvents() {
  this->engineState->handleEvents(&this->displayIsRunning);
}

/**
 * Not implemented but may want to add:
 * First check if it's time to update. If it is, reset the time since last
 * update.
 *
 * @param None
 * @return None
 */
void DisplayEngine::update() { this->engineState->update(); }

/**
 * Check current state and call that state's function to render.
 *
 * @param None
 * @return None
 */
void DisplayEngine::renderState() { this->engineState->render(); }

/**
 * Free SDL resources and quit.
 *
 * @param None
 * @return None
 */
void DisplayEngine::clean() {
  SDL_DestroyWindow(this->displayGlobal.window);
  SDL_DestroyRenderer(this->displayGlobal.renderer);
  SDL_Quit();
  LOG(INFO) << "DisplayEngine cleaned";
}
