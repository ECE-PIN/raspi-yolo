#ifndef STATE_H
#define STATE_H

#include <memory>
#include <vector>

#include "../display_global.h"
#include "../display_handler.h"
#include "../elements/button.h"
#include "../elements/composite_element.h"
#include "../elements/container.h"
#include "../elements/scroll_box.h"
#include "../elements/text.h"
#include "../engine_state.h"

class State {
public:
  State(const DisplayGlobal& displayGlobal, const EngineState& state);
  virtual void handleEvents(bool* displayIsRunning);
  virtual void update();
  virtual void render() const = 0;
  virtual void enter();
  virtual void exit() = 0;

  EngineState getCurrentState();
  void setCurrentState(EngineState currentState);

  bool checkStateChange();

protected:
  struct DisplayGlobal displayGlobal;
  const EngineState defaultState;
  EngineState currentState;

  std::shared_ptr<Container> rootElement;
  SDL_Surface* windowSurface = nullptr;

  DisplayHandler& displayHandler;
};

#endif
