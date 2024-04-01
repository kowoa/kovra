#pragma once

#include "SDL_video.h"
#include "instance.hpp"

namespace kovra {
class Surface
{
  public:
    Surface(const Instance &instance, SDL_Window *window);
    ~Surface();

    Surface(const Surface &) = delete;
    Surface &operator=(const Surface &) = delete;
    Surface(Surface &&) = delete;
    Surface &operator=(Surface &&) = delete;

    [[nodiscard]] const vk::SurfaceKHR &get() const noexcept
    {
        return surface.get();
    }

  private:
    vk::UniqueSurfaceKHR surface;
};
} // namespace kovra
