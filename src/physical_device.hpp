#pragma once

#include "queue.hpp"
#include <vulkan/vulkan.hpp>

namespace kovra {
class Surface;
class PhysicalDevice {
  public:
    PhysicalDevice(vk::PhysicalDevice physical_device, const Surface &surface);
    ~PhysicalDevice();

    [[nodiscard]] const vk::PhysicalDevice &get() const noexcept {
        return physical_device;
    }
    [[nodiscard]] const std::vector<QueueFamily> &
    get_queue_families() const noexcept {
        return queue_families;
    }
    [[nodiscard]] const DeviceFeatures &
    get_supported_features() const noexcept {
        return supported_features;
    }
    [[nodiscard]] QueueFamily get_graphics_queue_family() const;
    [[nodiscard]] QueueFamily get_present_queue_family() const;
    [[nodiscard]] bool supports_extensions(
        const std::vector<std::string> &extensions) const noexcept;

  private:
    vk::PhysicalDevice physical_device;
    std::string name;
    vk::PhysicalDeviceType type;
    vk::PhysicalDeviceLimits limits;
    std::vector<QueueFamily> queue_families;
    std::vector<std::string> supported_extensions;
    std::vector<vk::SurfaceFormatKHR> supported_surface_formats;
    std::vector<vk::PresentModeKHR> supported_present_modes;
    DeviceFeatures supported_features;
};
} // namespace kovra
