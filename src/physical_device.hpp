#pragma once

#include "queue.hpp"
#include <vulkan/vulkan.hpp>

namespace kovra {
class Instance;
class Surface;
struct DeviceFeatures {
    bool ray_tracing_pipeline;
    bool acceleration_structure;
    bool runtime_descriptor_array;
    bool buffer_device_address;
    bool dynamic_rendering;
    bool synchronization2;
};
class PhysicalDevice {
  public:
    PhysicalDevice(
        vk::PhysicalDevice physical_device, Instance &instance,
        const Surface &surface);
    [[nodiscard]] const vk::PhysicalDevice &get() const noexcept {
        return physical_device;
    }

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
