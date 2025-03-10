#ifndef SCROLL_BOX_H
#define SCROLL_BOX_H

#include <SDL2/SDL.h>
#include <chrono>
#include <memory>
#include <vector>

#include "../../../../food_item.h"
#include "../display_global.h"
#include "panel.h"
#include "sort_method.h"

/**
 * An element that contains a column of panels and allows the user to scroll through them
 * with the mousewheel.
 *
 * @see Panel
 */
class ScrollBox : public CompositeElement {
public:
  ScrollBox(struct DisplayGlobal displayGlobal,
            const SDL_Rect& boundaryRectangle,
            const std::string& logFile);

  void setPanelHeight(int panelHeight);
  void addPanel(std::unique_ptr<Panel> panel, SDL_Rect containingRectangle);

  void updateSelf() override;
  void renderSelf() const override;
  void handleEventSelf(const SDL_Event& event);

  void setSortMethod(SortMethod sortMethod);

private:
  void refreshPanels();
  void scrollUp();
  void scrollDown();

  std::chrono::steady_clock::time_point previousUpdate;
  std::chrono::steady_clock::time_point currentUpdate;
  int panelHeight;
  int topPanelPosition = 0;
  int scrollAmount     = 4;

  SortMethod sortMethod = SortMethod::LOW_TO_HIGH;
};

#endif
