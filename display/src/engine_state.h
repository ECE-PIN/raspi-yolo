#ifndef ENGINE_STATE_H
#define ENGINE_STATE_H

#include <string>

enum class EngineState {
  SCANNING,
  ITEM_LIST,
  ZERO_WEIGHT,
  CANCEL_SCAN_CONFIRMATION,
  SCAN_SUCCESS
};

std::string engineStateToString(EngineState engineState);

#endif
