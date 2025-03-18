#include <SDL2/SDL.h>
#include <glog/logging.h>

#include "../../../endpoints.h"
#include "display_engine.h"
#include "display_global.h"
#include "engine_entry.h"

/**
 * Function called when in the new SDL process. Kicks off the SDL "display" loop.
 *
 * @param None
 * @return None
 */
void engineEntry(const zmqpp::context& context, const std::string& engineEndpoint) {
  LOG(INFO) << "SDL display process started successfully";

  struct DisplayGlobal displayGlobal;

  // Initialize the displayEngine
  DisplayEngine displayEngine("Display", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              1024, 600, true, displayGlobal, context,
                              ExternalEndpoints::displayEndpoint, engineEndpoint);

  while (displayEngine.running()) {
    displayEngine.handleEvents();
    displayEngine.checkState();
    displayEngine.checkKeystates();
    displayEngine.checkState();
    displayEngine.update();
    displayEngine.renderState();
  }
  displayEngine.clean();
}
