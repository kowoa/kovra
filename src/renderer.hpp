#pragma once

#include <SDL2/SDL.h>
#include <vulkan/vulkan.hpp>

#include "context.hpp"
// #include "swapchain.hpp"

namespace kovra {
class Renderer {
  public:
    Renderer(SDL_Window *window);
    void draw_frame();

  private:
    Context context;
    // Swapchain swapchain;
};
} // namespace kovra
