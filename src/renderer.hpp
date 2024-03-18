#pragma once

#include "swapchain.hpp"

#include "frame.hpp"
#include "material.hpp"

namespace kovra {
class Renderer {
  public:
    explicit Renderer(SDL_Window *window);
    ~Renderer();

    void draw_frame();

  private:
    static constexpr const uint32_t FRAME_OVERLAP = 2;

    Context context;
    Swapchain swapchain;

    uint32_t frame_number;
    std::vector<std::unique_ptr<Frame>> frames;

    [[nodiscard]] const Frame &get_current_frame() const noexcept {
        return *frames[frame_number % frames.size()];
    }
};
} // namespace kovra
