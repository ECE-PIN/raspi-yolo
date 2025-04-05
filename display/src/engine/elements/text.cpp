#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <glog/logging.h>
#include <iostream>
#include <string>

#include "../sdl_debug.h"
#include "element.h"
#include "text.h"

Text::Text(struct DisplayGlobal displayGlobal,
           const SDL_Rect& boundaryRectangle,
           const std::string& fontPath,
           const std::string& content,
           int fontSize,
           SDL_Color color)
    : content(content), fontSize(fontSize), color(color) {
  this->displayGlobal = displayGlobal;

  setupPosition(boundaryRectangle);

  this->font = TTF_OpenFont(fontPath.c_str(), this->fontSize);
  if (this->font == NULL) {
    LOG(FATAL) << "Text failed to open font";
  }

  SDL_Surface* textSurface =
      TTF_RenderText_Solid(this->font, this->content.c_str(), this->color);
  this->texture = SDL_CreateTextureFromSurface(this->displayGlobal.renderer, textSurface);
  SDL_FreeSurface(textSurface);

  // Set width and height
  SDL_QueryTexture(this->texture, NULL, NULL, &this->boundaryRectangle.w,
                   &this->boundaryRectangle.h);
}

Text::~Text() { TTF_CloseFont(this->font); }

void Text::setContent(const std::string& content) {
  this->content = content;

  if (this->texture) {
    SDL_DestroyTexture(this->texture);
    this->texture = nullptr;
  }

  SDL_Surface* textSurface =
      TTF_RenderText_Solid(this->font, this->content.c_str(), this->color);
  this->texture = SDL_CreateTextureFromSurface(this->displayGlobal.renderer, textSurface);
  SDL_FreeSurface(textSurface);
}

void Text::render() const {
  SDL_RenderCopy(this->displayGlobal.renderer, this->texture, NULL,
                 &this->boundaryRectangle);
}

void Text::handleEvent(const SDL_Event& event) {}
