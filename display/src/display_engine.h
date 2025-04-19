#ifndef DISPLAY_ENGINE_H
#define DISPLAY_ENGINE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <memory>
#include <string>
#include <vector>
#include <zmqpp/zmqpp.hpp>

#include "../../logger.h"

#include "states/cancel_scan_confirmation.h"
#include "states/item_list.h"
#include "states/scan_success.h"
#include "states/scanning.h"
#include "states/zero_weight.h"

#include "display_global.h"
#include "display_handler.h"
#include "engine_state.h"

class DisplayEngine {
public:
  DisplayEngine(const char* windowTitle,
                int windowXPosition,
                int windowYPosition,
                int screenWidth,
                int screenHeight,
                bool fullscreen,
                const zmqpp::context& context);
  void start();

private:
  Logger logger;
  struct DisplayGlobal displayGlobal;
  State* engineState       = nullptr;
  EngineState currentState = EngineState::ITEM_LIST;
  bool displayIsRunning    = false;

  // States
  std::unique_ptr<Scanning> scanning;
  std::unique_ptr<ItemList> itemList;
  std::unique_ptr<ZeroWeight> zeroWeight;
  std::unique_ptr<CancelScanConfirmation> cancelScanConfirmation;
  std::unique_ptr<ScanSuccess> scanSuccess;

  SDL_Window* setupWindow(const char* windowTitle,
                          int windowXPosition,
                          int windowYPosition,
                          int screenWidth,
                          int screenHeight,
                          bool fullscreen);
  void initializeEngine(SDL_Window* window);

  void handleStateChange();
  void handleEvents();
  void update();
  void renderState();
  void clean();
};

#endif
