#ifndef CANCEL_SCAN_CONFIRMATION_H
#define CANCEL_SCAN_CONFIRMATION_H

#include "../display_global.h"
#include "state.h"

class CancelScanConfirmation : public State {
public:
  CancelScanConfirmation(struct DisplayGlobal displayglobal);
  void render() const override;

private:
  Logger logger;
};

#endif
