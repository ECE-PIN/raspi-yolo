#ifndef TEXT_H
#define TEXT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>

#include "../display_global.h"
#include "element.h"

/**
 * Represents a string of characters that can be rendered.
 */
class Text : public Element {
public:
  Text();
  Text(struct DisplayGlobal displayGlobal,
       const std::string& fontPath,
       const std::string& content,
       int fontSize,
       SDL_Color color,
       SDL_Rect rectangle);
  ~Text();

  void setContent(const std::string& newContent);
  void update() override;
  void render() const override;

private:
  TTF_Font* font;
  std::string content;
  int fontSize;
  SDL_Color color;
  SDL_Texture* texture;
};

#endif
