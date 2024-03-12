#include "device.hpp"

namespace kovra {
DeviceFeatures::DeviceFeatures(const vk::PhysicalDevice &physical_device) {
    // Check if the physical device supports the specified features
    auto features = physical_device.getFeatures2<
        vk::PhysicalDeviceFeatures2,                   // Vulkan 1.0
        vk::PhysicalDeviceVulkan12Features,            // Vulkan 1.2
        vk::PhysicalDeviceVulkan13Features,            // Vulkan 1.3
        vk::PhysicalDeviceBufferDeviceAddressFeatures, // VK_KHR_buffer_device_address
        vk::PhysicalDeviceDynamicRenderingFeatures, // VK_KHR_dynamic_rendering
        vk::PhysicalDeviceSynchronization2Features, // VK_KHR_synchronization2
        vk::PhysicalDeviceDescriptorIndexingFeatures, // VK_EXT_descriptor_indexing
        vk::PhysicalDeviceRayTracingPipelineFeaturesKHR, // VK_KHR_ray_tracing_pipeline
        vk::PhysicalDeviceAccelerationStructureFeaturesKHR>(); // VK_KHR_acceleration_structure

    // Features from Vulkan 1.2
    const vk::PhysicalDeviceVulkan12Features &features12 =
        features.get<vk::PhysicalDeviceVulkan12Features>();
    // Features from Vulkan 1.3
    const vk::PhysicalDeviceVulkan13Features &features13 =
        features.get<vk::PhysicalDeviceVulkan13Features>();
    // Features from VK_KHR_ray_tracing_pipeline
    const vk::PhysicalDeviceRayTracingPipelineFeaturesKHR
        &ray_tracing_features =
            features.get<vk::PhysicalDeviceRayTracingPipelineFeaturesKHR>();
    // Features from VK_KHR_acceleration_structure
    const vk::PhysicalDeviceAccelerationStructureFeaturesKHR
        &acceleration_structure_features =
            features.get<vk::PhysicalDeviceAccelerationStructureFeaturesKHR>();

    dynamic_rendering = features13.dynamicRendering;
    synchronization2 = features13.synchronization2;
    runtime_descriptor_array = features12.descriptorIndexing;
    buffer_device_address = features12.bufferDeviceAddress;
    ray_tracing_pipeline = ray_tracing_features.rayTracingPipeline;
    acceleration_structure =
        acceleration_structure_features.accelerationStructure;
}
bool DeviceFeatures::is_compatible_with(const DeviceFeatures &other) const {
    return (!other.dynamic_rendering || dynamic_rendering) &&
           (!other.synchronization2 || synchronization2) &&
           (!other.runtime_descriptor_array || runtime_descriptor_array) &&
           (!other.buffer_device_address || buffer_device_address) &&
           (!other.ray_tracing_pipeline || ray_tracing_pipeline) &&
           (!other.acceleration_structure || acceleration_structure);
}
} // namespace kovra
