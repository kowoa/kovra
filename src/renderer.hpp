#pragma once

#include "swapchain.hpp"

namespace kovra {
class Renderer {
  public:
    Renderer(SDL_Window *window);
    ~Renderer();
    void draw_frame();

  private:
    Context context;
    Swapchain swapchain;
};
} // namespace kovra
