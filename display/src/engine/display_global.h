#ifndef DISPLAY_GLOBAL_H
#define DISPLAY_GLOBAL_H

#include <SDL2/SDL.h>
#include <string>

struct DisplayGlobal {
  SDL_Renderer* renderer;
  SDL_Window* window;
  static const std::string futuramFontPath;
  static const std::string logger;
};

#endif
