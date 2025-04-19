#include "engine_state.h"

std::string engineStateToString(EngineState engineState) {
  switch (engineState) {
  case EngineState::SCANNING:
    return "SCANNING";
  case EngineState::ITEM_LIST:
    return "ITEM_LIST";
  case EngineState::ZERO_WEIGHT:
    return "ZERO_WEIGHT";
  case EngineState::CANCEL_SCAN_CONFIRMATION:
    return "CANCEL_SCAN_CONFIRMATION";
  case EngineState::SCAN_SUCCESS:
    return "SCAN_SUCCESS";
  default:
    return "UNKNOWN";
  }
}
