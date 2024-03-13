#pragma once

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#include "physical_device.hpp"
#include "surface.hpp"
#include <memory>
#include <vulkan/vulkan.hpp>

namespace kovra {
class Instance {
  public:
    Instance(SDL_Window *window);
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
