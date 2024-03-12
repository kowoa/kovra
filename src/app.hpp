#pragma once

#include "SDL_video.h"

namespace kovra {
class App {
  public:
    App();
    void run();

  private:
    SDL_Window *window;
};
} // namespace kovra
