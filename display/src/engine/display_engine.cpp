#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>
#include <glog/logging.h>
#include <iostream>

#include "../../../endpoints.h"

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
 * @param displayGlobal Global display variables
 */
DisplayEngine::DisplayEngine(const char* windowTitle,
                             int windowXPosition,
                             int windowYPosition,
                             int screenWidth,
                             int screenHeight,
                             bool fullscreen,
                             struct DisplayGlobal displayGlobal,
                             const zmqpp::context& context,
                             const std::string& displayEndpoint,
                             const std::string& engineEndpoint)
    : replySocket(context, zmqpp::socket_type::reply),
      requestSocket(context, zmqpp::socket_type::request),
      DISPLAY_ENDPOINT(displayEndpoint), ENGINE_ENDPOINT(engineEndpoint),
      logger("display_engine.txt") {
  this->logger.log("Constructing display engine");
  // Setup sockets
  try {
    this->requestSocket.connect(this->DISPLAY_ENDPOINT);
    this->replySocket.bind(this->ENGINE_ENDPOINT);
  } catch (const zmqpp::exception& e) {
    LOG(FATAL) << e.what();
  }

  this->displayGlobal = displayGlobal;
  this->displayGlobal.window =
      setupWindow(windowTitle, windowXPosition, windowYPosition, screenWidth,
                  screenHeight, fullscreen); // Setup the SDL display window

  initializeEngine(this->displayGlobal.window);

  // Initialize states
  this->scanning = std::make_unique<Scanning>(this->displayGlobal);
  this->itemList = std::make_unique<ItemList>(this->displayGlobal);

  displayIsRunning = true;
  this->logger.log("Engine is constructed and now running");
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

/**
 * Check if the current state has requested a state switch.
 *
 * @param None
 * @return None
 */
void DisplayEngine::checkState() {
  switch (this->engineState) {
  case EngineState::SCANNING:
    this->engineState = this->scanning->getCurrentState();
    this->scanning->setCurrentState(EngineState::SCANNING);
    break;

  case EngineState::ITEM_LIST:
    this->engineState = this->itemList->getCurrentState();

    if (this->engineState == EngineState::SCANNING) {
      startSignalToDisplay();
    }

    this->itemList->setCurrentState(EngineState::ITEM_LIST);
    break;

  default:
    std::cout << engineStateToString(this->engineState) << std::endl;
    LOG(FATAL) << "Invalid state";
    break;
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
  switch (this->engineState) {
  case EngineState::SCANNING:
    {
      this->scanning->handleEvents(&this->displayIsRunning);

      bool gotMessageFromDisplay = false;
      std::string messageFromDisplay;
      gotMessageFromDisplay = this->replySocket.receive(messageFromDisplay, true);
      if (gotMessageFromDisplay == false) {
        break;
      }
      if (messageFromDisplay == Messages::ITEM_DETECTION_SUCCEEDED) {
        this->logger.log("New food item received, switching to item list state");
        this->engineState = EngineState::ITEM_LIST;
        this->replySocket.send(Messages::AFFIRMATIVE);
      }
      else {
        LOG(FATAL) << "Invalid message received from display";
      }
      break;
    }

  case EngineState::ITEM_LIST:
    this->itemList->handleEvents(&this->displayIsRunning);
    break;

  default:
    LOG(FATAL) << "Invalid state";
    break;
  }
}

/**
 * Check the current state of the display and call that state's method to check the
 * key states
 *
 * @param None
 * @return None
 */
void DisplayEngine::checkKeystates() {
  switch (this->engineState) {
  case EngineState::SCANNING:
    this->engineState = this->scanning->checkKeystates();
    break;

  case EngineState::ITEM_LIST:
    this->engineState = this->itemList->checkKeystates();
    break;

  default:
    LOG(FATAL) << "Invalid state";
    break;
  }
}

/**
 * Not implemented but may want to add:
 * First check if it's time to update. If it is, reset the time since last
 * update.
 *
 * @param None
 * @return None
 */
void DisplayEngine::update() {
  switch (this->engineState) {
  case EngineState::SCANNING:
    this->scanning->update();
    break;

  case EngineState::ITEM_LIST:
    this->itemList->update();
    break;

  default:
    LOG(FATAL) << "Invalid state";
    break;
  }
}

/**
 * Check current state and call that state's function to render.
 *
 * @param None
 * @return None
 */
void DisplayEngine::renderState() {
  switch (this->engineState) {
  case EngineState::SCANNING:
    this->scanning->render();
    break;

  case EngineState::ITEM_LIST:
    this->itemList->render();
    break;

  default:
    LOG(FATAL) << "Invalid state";
    break;
  }
}

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

void DisplayEngine::startSignalToDisplay() {
  this->logger.log("Scan initialized, sending start signal to display");
  try {
    this->requestSocket.send(Messages::START_SCAN);
    std::string response;
    this->requestSocket.receive(response);
    if (response == Messages::AFFIRMATIVE) {
      this->logger.log("Start signal successfully sent to display, in scanning state");
    }
    else {
      LOG(FATAL) << "Invalid response received from display";
    }
  } catch (const zmqpp::exception& e) {
    LOG(FATAL) << e.what();
  }
}
