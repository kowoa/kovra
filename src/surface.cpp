#include "surface.hpp"
#include "SDL_vulkan.h"
#include "spdlog/spdlog.h"

namespace kovra {
Surface::Surface(const Instance &instance, SDL_Window *window) {
    spdlog::debug("Surface::Surface()");
    VkSurfaceKHR vk_surface;
    if (!SDL_Vulkan_CreateSurface(window, instance.get(), &vk_surface)) {
        throw std::runtime_error("Failed to create Vulkan surface");
    }
    surface = vk::UniqueSurfaceKHR{vk_surface, instance.get()};
}
} // namespace kovra
