#ifndef PAUSE_MENU_H
#define PAUSE_MENU_H

#include <SDL2/SDL_ttf.h>
#include <memory>
#include <string>

#include "../display_global.h"
#include "../elements/button.h"
#include "../elements/text.h"
#include "../engine_state.h"
#include "state.h"

/**
 * Pause menu state. Entered when pausing from gameplay. Allows resuming of scanning or
 * return to main menu.
 */
class PauseMenu : public State {
public:
  PauseMenu(struct DisplayGlobal);
  void render() const override;

private:
  Logger logger;
};

#endif
