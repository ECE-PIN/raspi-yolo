#ifndef NUMBER_SETTING_H
#define NUMBER_SETTING_H

#include <vector>

#include "button.h"
#include "element.h"
#include "text.h"

/**
 * Element that contains two buttons with some text sandwiched between them.
 * Used to display a setting and allow the user to change said setting.
 */
class NumberSetting : public CompositeElement {
public:
  NumberSetting(struct DisplayGlobal displayGlobal,
                const SDL_Rect& boundaryRectangle,
                int settingId,
                const std::string& logFile);

  void setSettingId(const int& newSettingId);
  void updateSelf() override;
  void handleEventSelf(const SDL_Event& event) override;

private:
  int settingValue = -1;
  int settingId    = -1;
};

#endif
