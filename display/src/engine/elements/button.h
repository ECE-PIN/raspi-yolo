#ifndef BUTTON_H
#define BUTTON_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <functional>
#include <memory>
#include <string>

#include "../display_global.h"
#include "composite_element.h"
#include "element.h"
#include "text.h"

/**
 * Clickable button containing centered text. Can be set to perform an action upon click.
 */
class Button : public CompositeElement {
public:
  Button(struct DisplayGlobal displayGlobal,
         const SDL_Rect& boundaryRectangle,
         const std::string& textContent,
         const SDL_Point& textPadding,
         std::function<void()> callback,
         const std::string& logFile);

  Button(struct DisplayGlobal displayGlobal,
         const SDL_Rect& boundaryRectangle,
         const std::string& textContent,
         const SDL_Point& textPadding,
         const std::string& notifyMessage,
         const std::string& logFile);

  void initialize();
  void updateSelf() override;
  void renderSelf() const override;
  void handleEventSelf(const SDL_Event& event) override;

private:
  std::string textContent;
  std::function<void()> onClick;
  SDL_Color backgroundColor;
  SDL_Color defaultColor;
  SDL_Color hoveredColor;
  SDL_Point textPadding = {0, 0};
  std::string notifyMessage;
};

#endif
