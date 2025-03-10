#ifndef STATE_H
#define STATE_H

#include <memory>
#include <vector>

#include "../display_global.h"
#include "../elements/button.h"
#include "../elements/composite_element.h"
#include "../elements/container.h"
#include "../elements/scroll_box.h"
#include "../elements/text.h"
#include "../engine_state.h"

class State {
public:
  virtual void handleEvents(bool* displayIsRunning);
  virtual void update();
  virtual void render() const = 0;
  EngineState getCurrentState();
  void setCurrentState(EngineState currentState);

protected:
  EngineState currentState;
  struct DisplayGlobal displayGlobal;
  std::shared_ptr<Container> rootElement;
};

#endif
