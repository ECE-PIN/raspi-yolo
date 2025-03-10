#ifndef DISPLAY_ENGINE_H
#define DISPLAY_ENGINE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <memory>
#include <string>
#include <vector>
#include <zmqpp/zmqpp.hpp>

#include "../../../logger.h"

#include "states/item_list.h"
#include "states/scanning.h"

#include "display_global.h"
#include "engine_state.h"

/**
 * Class containing all objects related to the engine.
 */
class DisplayEngine {
public:
  DisplayEngine(const char* windowTitle,
                int windowXPosition,
                int windowYPosition,
                int screenWidth,
                int screenHeight,
                bool fullscreen,
                struct DisplayGlobal displayGlobal,
                const zmqpp::context& context,
                const std::string& displayEndpoint,
                const std::string& engineEndpoint);

  SDL_Window* setupWindow(const char* windowTitle,
                          int windowXPosition,
                          int windowYPosition,
                          int screenWidth,
                          int screenHeight,
                          bool fullscreen);
  void initializeEngine(SDL_Window* window);

  void checkState();
  void handleEvents();
  void checkKeystates();
  void update();

  void renderState();
  void clean();
  bool running() { return displayIsRunning; }

private:
  struct DisplayGlobal displayGlobal;
  Logger logger;

  zmqpp::socket replySocket;
  zmqpp::socket requestSocket;

  const std::string& DISPLAY_ENDPOINT;
  const std::string& ENGINE_ENDPOINT;

  EngineState engineState = EngineState::ITEM_LIST;

  // States
  std::unique_ptr<Scanning> scanning;
  std::unique_ptr<ItemList> itemList;

  bool displayIsRunning = false;

  void startSignalToDisplay();
};

#endif
