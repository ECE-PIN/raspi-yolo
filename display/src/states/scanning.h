#ifndef SCANNING_H
#define SCANNING_H

#include <SDL2/SDL.h>
#include <memory>
#include <string>

#include "../display_global.h"
#include "../elements/bird.h"
#include "../elements/obstacle_pair.h"
#include "../engine_state.h"
#include "state.h"

/**
 * Scanning state. State entered when user presses scan new item button in main menu.
 */
class Scanning : public State {
public:
  Scanning(const DisplayGlobal& displayGlobal, const EngineState& state);
  void handleEvents(bool* displayIsRunning) override;
  void update() override;
  void render() const override;
  void exit() override;

private:
  Logger logger;

  Bird* birdPtr = nullptr;
  std::vector<ObstaclePair*> obstaclePairs;
  int score = 0;

  void initializeObstacles();
  std::vector<SDL_Rect> getBoundaryRectangles();

  void handleBirdCollision();
};

#endif
