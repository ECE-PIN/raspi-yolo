#ifndef ITEM_LIST_H
#define ITEM_LIST_H

#include <chrono>

#include "../display_global.h"
#include "../elements/element_mediator.h"
#include "state.h"

/**
 * State where all stored food items are displayed. Items are displayed in a scrollbox and
 * a dropdown is used to change the ordering of the food items in the scroll box.
 */
class ItemList : public State {
public:
  ItemList(const DisplayGlobal& displayGlobal, const EngineState& state);
  void handleEvents(bool* displayIsRunning) override;
  void render() const override;
  void exit() override;

private:
  SDL_Point mousePosition;
  std::shared_ptr<Mediator> mediator;

  const int SCROLLBOX_WIDTH = 600;
};

#endif
