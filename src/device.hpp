#pragma once

#include <vulkan/vulkan.hpp>

namespace kovra {
// Forward declarations
class Instance;
class PhysicalDevice;
class DeviceFeatures;
class QueueFamily;
class Queue;

class Device {
  public:
    Device(const PhysicalDevice &physical_device);
    [[nodiscard]] const vk::Device &get() const noexcept {
        return device.get();
    }

  private:
    vk::UniqueDevice device;
};
class DeviceFeatures {
  public:
    DeviceFeatures(const vk::PhysicalDevice &physical_device);
    bool is_compatible_with(const DeviceFeatures &other) const;
    bool dynamic_rendering;
    bool synchronization2;
    bool runtime_descriptor_array;
    bool buffer_device_address;
    bool ray_tracing_pipeline;
    bool acceleration_structure;
};
} // namespace kovra
