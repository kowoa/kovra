#pragma once

#include "swapchain.hpp"

#include "frame.hpp"

namespace kovra {
class Renderer {
  public:
    Renderer(SDL_Window *window);
    ~Renderer();
    void draw_frame();

  private:
    Context context;
    Swapchain swapchain;

    uint32_t frame_number;
    std::vector<std::unique_ptr<Frame>> frames;
};
} // namespace kovra
