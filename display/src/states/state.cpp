#include <iostream>

#include "../display_global.h"
#include "state.h"

void State::handleEvents(bool* displayIsRunning) {
  SDL_Event event;
  while (SDL_PollEvent(&event) != 0) { // While there are events in the queue
    if (event.type == SDL_QUIT) {
      *displayIsRunning = false;
      break;
    }
    else {
      this->rootElement->handleEvent(event);
    }
  }
}

void State::update() { this->rootElement->update(); }

EngineState State::getCurrentState() { return this->currentState; }

void State::setCurrentState(EngineState currentState) {
  this->currentState = currentState;
}
