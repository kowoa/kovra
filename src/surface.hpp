#pragma once

#include "SDL_video.h"
#include "instance.hpp"
#include <vulkan/vulkan.hpp>

namespace kovra {
class Surface {
  public:
    Surface(const Instance &instance, SDL_Window *window);

    [[nodiscard]] const vk::SurfaceKHR &get() const noexcept {
        return surface.get();
    }

  private:
    vk::UniqueSurfaceKHR surface;
};
} // namespace kovra
