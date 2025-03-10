#ifndef SCANNING_H
#define SCANNING_H

#include <SDL2/SDL.h>
#include <memory>
#include <string>

#include "../display_global.h"
#include "../engine_state.h"
#include "state.h"

/**
 * Scanning state. State entered when user presses scan new item button in main menu.
 */
class Scanning : public State {
public:
  Scanning(struct DisplayGlobal displayGlobal);
  EngineState checkKeystates();
  void render() const override;

private:
  Logger logger;
};

#endif
