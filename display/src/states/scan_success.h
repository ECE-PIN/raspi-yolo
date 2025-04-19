#ifndef SCAN_SUCCESS_H
#define SCAN_SUCCESS_H

#include "state.h"

class ScanSuccess : public State {
public:
  ScanSuccess(struct DisplayGlobal& displayGlobal, const EngineState& state);
  void render() const override;
  void enter() override;
  void exit() override;

private:
  Logger logger;
  FoodItem foodItem;
  bool correctItem = false;
  std::shared_ptr<Panel> scannedItemPanel;
};

#endif
