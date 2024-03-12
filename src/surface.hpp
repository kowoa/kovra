#pragma once

#include "SDL.h"
#include "SDL_video.h"

namespace kovra {
class Instance;
class Surface {
  public:
    Surface(const Instance &instance, SDL_Window *window);
};
} // namespace kovra
