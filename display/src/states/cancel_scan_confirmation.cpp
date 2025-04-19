#include <iostream>

#include "../log_files.h"
#include "cancel_scan_confirmation.h"

CancelScanConfirmation::CancelScanConfirmation(const DisplayGlobal& displayGlobal,
                                               const EngineState& state)
    : State(displayGlobal, state), logger(LogFiles::CANCEL_SCAN_CONFIRMATION) {
  this->logger.log("Constructing cancel scan confirmation state");

  const char* cancelPromptContent    = "Are you sure you want to cancel the scan?";
  SDL_Color cancelPromptColor        = {0, 255, 0, 255}; // Green
  SDL_Rect cancelPromptRectangle     = {0, 100, 0, 0};
  std::unique_ptr<Text> cancelPrompt = std::make_unique<Text>(
      this->displayGlobal, cancelPromptRectangle, DisplayGlobal::futuramFontPath,
      cancelPromptContent, 24, cancelPromptColor);

  cancelPrompt->setCenteredHorizontal();
  this->rootElement->addElement(std::move(cancelPrompt));

  SDL_Rect yesButtonRectangle       = {0, 150, 0, 0};
  std::unique_ptr<Button> yesButton = std::make_unique<Button>(
      this->displayGlobal, yesButtonRectangle, "Yes", SDL_Point{10, 10},
      [this]() { this->currentState = EngineState::ITEM_LIST; },
      LogFiles::CANCEL_SCAN_CONFIRMATION);
  yesButton->setCenteredHorizontal();
  rootElement->addElement(std::move(yesButton));

  SDL_Rect noButtonRectangle       = {0, 200, 0, 0};
  std::unique_ptr<Button> noButton = std::make_unique<Button>(
      this->displayGlobal, noButtonRectangle, "No", SDL_Point{10, 10},
      [this]() { this->currentState = EngineState::SCANNING; },
      LogFiles::CANCEL_SCAN_CONFIRMATION);
  noButton->setCenteredHorizontal();
  rootElement->addElement(std::move(noButton));

  this->logger.log("Constructed cancel scan confirmation state");
}

/**
 * Render all elements.
 *
 * @param None
 * @return None
 */
void CancelScanConfirmation::render() const {
  SDL_SetRenderDrawColor(this->displayGlobal.renderer, 0, 0, 0, 255); // Black background
  SDL_RenderClear(this->displayGlobal.renderer);
  this->rootElement->render();
  SDL_RenderPresent(this->displayGlobal.renderer);
}

void CancelScanConfirmation::exit() {
  if (this->currentState == EngineState::ITEM_LIST) {
    this->displayHandler.scanCancelledToVision();
  }
}
