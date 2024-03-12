#pragma once

#include <vulkan/vulkan.hpp>

namespace kovra {
class Device {};
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
