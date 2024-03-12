#pragma once

#include "SDL_video.h"
#include "renderer.hpp"
#include <memory>

namespace kovra {
class App {
  public:
    App();
    ~App();
    void run();

  private:
    SDL_Window *window;
    std::unique_ptr<Renderer> renderer;
};
} // namespace kovra
