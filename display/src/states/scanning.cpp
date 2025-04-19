#include <SDL2/SDL.h>
#include <assert.h>
#include <glog/logging.h>
#include <iostream>
#include <string>

#include "../display_global.h"
#include "../elements/bird.h"
#include "../elements/loading_bar.h"
#include "../elements/obstacle.h"
#include "../elements/obstacle_pair.h"
#include "../elements/text.h"
#include "../log_files.h"
#include "scanning.h"

/**
 * @param displayGlobal Global variables
 */
Scanning::Scanning(const DisplayGlobal& displayGlobal, const EngineState& state)
    : State(displayGlobal, state), logger(LogFiles::SCANNING) {
  this->logger.log("Constructing scanning state");

  this->logger.log("Constructing scan message");
  const std::string progressMessageContent = "Scanning In Progress";
  const SDL_Color progressMessageColor     = {0, 255, 0, 255}; // Green
  const SDL_Rect progressMessageRectangle  = {0, 100, 0, 0};
  std::unique_ptr<Text> progressMessage    = std::make_unique<Text>(
      this->displayGlobal, progressMessageRectangle, DisplayGlobal::futuramFontPath,
      progressMessageContent, 24, progressMessageColor);
  progressMessage->setCenteredHorizontal();
  this->rootElement->addElement(std::move(progressMessage));
  this->logger.log("Scan message constructed");

  this->logger.log("Constructing cancel scan button");
  SDL_Rect cancelScanButtonRectangle       = {0, 150, 0, 0};
  std::unique_ptr<Button> cancelScanButton = std::make_unique<Button>(
      this->displayGlobal, cancelScanButtonRectangle, "Cancel Scan", SDL_Point{10, 10},
      [this]() { this->currentState = EngineState::CANCEL_SCAN_CONFIRMATION; },
      LogFiles::SCANNING);
  cancelScanButton->setCenteredHorizontal();
  rootElement->addElement(std::move(cancelScanButton));
  this->logger.log("Cancel scan button constructed");

  this->logger.log("Constructing loading bar");
  SDL_Rect loadingBarRectangle           = {0, 200, 200, 30};
  int loadingBarBorderThickness          = 3;
  float totalTimeSeconds                 = 20;
  float updatePeriodMs                   = 100;
  std::unique_ptr<LoadingBar> loadingBar = std::make_unique<LoadingBar>(
      this->displayGlobal, loadingBarRectangle, loadingBarBorderThickness,
      totalTimeSeconds, updatePeriodMs, LogFiles::SCANNING);
  loadingBar->setCenteredHorizontal();
  rootElement->addElement(std::move(loadingBar));
  this->logger.log("Loading bar constructed");

  this->logger.log("Constructing bird");
  SDL_Rect birdRectangle     = {0, 0, 32, 32};
  std::unique_ptr<Bird> bird = std::make_unique<Bird>(this->displayGlobal, birdRectangle);
  birdPtr                    = bird.get();
  rootElement->addElement(std::move(bird));
  this->logger.log("Bird constructed");

  initializeObstacles();

  this->logger.log("Constructed scanning state");
}

void Scanning::handleEvents(bool* displayIsRunning) {
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

  this->currentState = this->displayHandler.checkDetectionResults(this->currentState);
}

/**
 * Render all scanning elements.
 *
 * @param None
 * @return None
 */
void Scanning::render() const {
  SDL_SetRenderDrawColor(this->displayGlobal.renderer, 0, 0, 0, 255); // Black background
  SDL_RenderClear(this->displayGlobal.renderer);
  this->rootElement->render();
  SDL_RenderPresent(this->displayGlobal.renderer);
}

void Scanning::update() {
  this->rootElement->update();

  std::vector<SDL_Rect> boundaryRectangles = getBoundaryRectangles();
  this->rootElement->checkCollision(boundaryRectangles);
  handleBirdCollision();

  SDL_Rect birdRect = this->birdPtr->getBoundaryRectangle();
  for (const auto& obstaclePair : this->obstaclePairs) {
    if (obstaclePair->scored) {
      continue;
    }
    SDL_Rect obstaclePairRect = obstaclePair->getBoundaryRectangle();
    SDL_Rect topRect          = obstaclePair->getTopObstacleRect();
    SDL_Rect bottomRect       = obstaclePair->getBottomObstacleRect();

    if (birdRect.x > obstaclePairRect.x + obstaclePairRect.w) {
      this->score++;
      obstaclePair->scored = true;
      std::cout << "score: " << this->score << std::endl;
    }
  }
}

void Scanning::initializeObstacles() {
  const int windowWidth  = this->windowSurface->w;
  const int windowHeight = this->windowSurface->h;

  const int obstacleWidth         = 40;
  const int obstaclePairHeight    = 200;
  const int horizontalObstacleGap = 70;

  int totalObstaclePairs = windowWidth / (obstacleWidth + horizontalObstacleGap);

  totalObstaclePairs++;
  const int respawnOffset = windowWidth % (obstacleWidth + horizontalObstacleGap);

  int xPosition       = windowWidth;
  const int yPosition = windowHeight - obstaclePairHeight;

  for (int i = 0; i < totalObstaclePairs; i++) {
    SDL_Rect boundaryRectangle = {xPosition, yPosition, obstacleWidth,
                                  obstaclePairHeight};

    std::unique_ptr<ObstaclePair> obstaclePair =
        std::make_unique<ObstaclePair>(this->displayGlobal, boundaryRectangle,
                                       windowWidth, respawnOffset, LogFiles::SCANNING);

    obstaclePairs.push_back(obstaclePair.get());

    this->rootElement->addElement(std::move(obstaclePair));
    xPosition += obstacleWidth + horizontalObstacleGap;
  }
}

std::vector<SDL_Rect> Scanning::getBoundaryRectangles() {
  std::vector<SDL_Rect> boundaryRectangles;

  this->rootElement->addBoundaryRectangle(boundaryRectangles);

  return boundaryRectangles;
}

void Scanning::handleBirdCollision() {
  if (this->birdPtr->getHasCollided()) {
    this->birdPtr->setVelocity(Velocity{0, 0});
    for (const auto& obstaclePair : this->obstaclePairs) {
      obstaclePair->setVelocity(Velocity{0, 0});
    }
  }
}

void Scanning::exit() {}
