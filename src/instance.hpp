#pragma once

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#include "SDL_video.h"
#include <memory>
#include <vulkan/vulkan.hpp>

namespace kovra {
// Forward declarations
class Surface;
class PhysicalDevice;

class Instance {
  public:
    explicit Instance(SDL_Window *window);
    ~Instance();

    [[nodiscard]] const vk::Instance &get() const noexcept {
        return instance.get();
    }

    const std::vector<std::shared_ptr<PhysicalDevice>> &
    enumerate_physical_devices(const Surface &surface);

  private:
    vk::UniqueInstance instance;
    vk::UniqueDebugUtilsMessengerEXT debug_messenger;
    std::vector<std::shared_ptr<PhysicalDevice>> physical_devices;
};
} // namespace kovra
