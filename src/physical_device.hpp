#pragma once

#include <vulkan/vulkan.hpp>

namespace kovra {
class Instance;
class Surface;
class PhysicalDevice {
  public:
    PhysicalDevice(
        vk::PhysicalDevice physical_device, const Instance &instance,
        const Surface &surface);
    [[nodiscard]] const vk::PhysicalDevice &get() const noexcept {
        return physical_device;
    }

  private:
    vk::PhysicalDevice physical_device;
};
} // namespace kovra
