#pragma once

#include "surface.hpp"

namespace kovra {
// Forward declarations
class QueueFamily;

class DeviceFeatures
{
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

class PhysicalDevice
{
  public:
    PhysicalDevice(vk::PhysicalDevice physical_device, const Surface &surface);
    ~PhysicalDevice();

    [[nodiscard]] const vk::PhysicalDevice &get() const noexcept
    {
        return physical_device;
    }
    [[nodiscard]] const std::vector<QueueFamily> &get_queue_families(
    ) const noexcept
    {
        return queue_families;
    }
    [[nodiscard]] const DeviceFeatures &get_supported_features() const noexcept
    {
        return supported_features;
    }
    [[nodiscard]] vk::DeviceSize get_min_uniform_buffer_offset_alignment(
    ) const noexcept
    {
        return limits.minUniformBufferOffsetAlignment;
    }
    [[nodiscard]] vk::DeviceSize get_min_storage_buffer_offset_alignment(
    ) const noexcept
    {
        return limits.minStorageBufferOffsetAlignment;
    }
    [[nodiscard]] vk::SampleCountFlags get_sample_counts() const noexcept
    {
        return limits.framebufferColorSampleCounts &
               limits.framebufferDepthSampleCounts;
    }

    [[nodiscard]] QueueFamily get_graphics_queue_family() const;
    [[nodiscard]] QueueFamily get_present_queue_family() const;
    [[nodiscard]] QueueFamily get_transfer_queue_family() const;
    [[nodiscard]] QueueFamily get_compute_queue_family() const;

    [[nodiscard]] bool supports_extensions(
      const std::vector<std::string> &extensions
    ) const noexcept;

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
