#ifndef CANCEL_SCAN_CONFIRMATION_H
#define CANCEL_SCAN_CONFIRMATION_H

#include "../display_global.h"
#include "state.h"

class CancelScanConfirmation : public State {
public:
  CancelScanConfirmation(const DisplayGlobal& displayGlobal, const EngineState& state);
  void render() const override;
  void exit() override;

private:
  Logger logger;
};

#endif
