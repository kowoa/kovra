#include "physical_device.hpp"
#include "instance.hpp"
#include "spdlog/spdlog.h"
#include <vulkan/vulkan_structs.hpp>

namespace kovra {
vk::PhysicalDevice pick_physical_device(
    const std::vector<PhysicalDevice> physical_devices, const Surface &surface);

PhysicalDevice::PhysicalDevice(
    vk::PhysicalDevice physical_device, Instance &instance,
    const Surface &surface) {
    spdlog::info("PhysicalDevice::PhysicalDevice()");

    this->physical_device = physical_device;

    auto props = physical_device.getProperties();
    name = std::string{props.deviceName};
    type = props.deviceType;
    limits = props.limits;

    auto queue_family_props = physical_device.getQueueFamilyProperties();
    for (size_t i = 0; i < queue_family_props.size(); i++) {
        bool present_support =
            physical_device.getSurfaceSupportKHR(i, surface.get());
        queue_families.emplace_back(QueueFamily{
            static_cast<uint32_t>(i), queue_family_props[i], present_support});
    }

    auto extension_props = physical_device.enumerateDeviceExtensionProperties();
    for (const auto &extension : extension_props) {
        supported_extensions.emplace_back(extension.extensionName);
    }

    supported_surface_formats =
        physical_device.getSurfaceFormatsKHR(surface.get());

    supported_present_modes =
        physical_device.getSurfacePresentModesKHR(surface.get());

    auto features = physical_device.getFeatures2<
        vk::PhysicalDeviceFeatures2,                   // Vulkan 1.0
        vk::PhysicalDeviceVulkan13Features,            // Vulkan 1.3
        vk::PhysicalDeviceVulkan12Features,            // Vulkan 1.2
        vk::PhysicalDeviceBufferDeviceAddressFeatures, // VK_KHR_buffer_device_address
        vk::PhysicalDeviceDescriptorIndexingFeatures, // VK_EXT_descriptor_indexing
        vk::PhysicalDeviceDynamicRenderingFeatures, // VK_KHR_dynamic_rendering
        vk::PhysicalDeviceSynchronization2Features, // VK_KHR_synchronization2
        vk::PhysicalDeviceRayTracingPipelineFeaturesKHR, // VK_KHR_ray_tracing_pipeline
        vk::PhysicalDeviceAccelerationStructureFeaturesKHR>(); // VK_KHR_acceleration_structure
    supported_features = DeviceFeatures{
        features.features.get<vk::PhysicalDeviceRayTracingPipelineFeaturesKHR>()
            .rayTracingPipeline,
        features.get<vk::PhysicalDeviceAccelerationStructureFeaturesKHR>()
            .accelerationStructure,
        features.get<vk::PhysicalDeviceDescriptorIndexingFeatures>()
            .runtimeDescriptorArray,
        features.get<vk::PhysicalDeviceBufferDeviceAddressFeatures>()
            .bufferDeviceAddress,
        features.get<vk::PhysicalDeviceDynamicRenderingFeatures>()
            .dynamicRendering,
        features.get<vk::PhysicalDeviceSynchronization2Features>()
            .synchronization2};
}

vk::PhysicalDevice pick_physical_device(
    const std::vector<PhysicalDevice> physical_devices,
    const Surface &surface) {
    // Loop through each physical device
    for (const auto &physical_device : physical_devices) {
        // Check if the physical device supports the surface
        bool surface_supported =
            physical_device.get().getSurfaceSupportKHR(0, surface.get());
        // Check if the physical device supports the specified features
        auto features =
            physical_device.get()
                .getFeatures2<
                    vk::PhysicalDeviceFeatures2,        // Vulkan 1.0
                    vk::PhysicalDeviceVulkan13Features, // Vulkan 1.3
                    vk::PhysicalDeviceVulkan12Features, // Vulkan 1.2
                    vk::PhysicalDeviceBufferDeviceAddressFeatures, // VK_KHR_buffer_device_address
                    vk::PhysicalDeviceDynamicRenderingFeatures, // VK_KHR_dynamic_rendering
                    vk::PhysicalDeviceSynchronization2Features, // VK_KHR_synchronization2
                    vk::PhysicalDeviceDescriptorIndexingFeatures, // VK_EXT_descriptor_indexing
                    vk::PhysicalDeviceRayTracingPipelineFeaturesKHR, // VK_KHR_ray_tracing_pipeline
                    vk::PhysicalDeviceAccelerationStructureFeaturesKHR>(); // VK_KHR_acceleration_structure

        // Features from Vulkan 1.3
        const vk::PhysicalDeviceVulkan13Features &features13 =
            features.get<vk::PhysicalDeviceVulkan13Features>();
        // Features from Vulkan 1.2
        const vk::PhysicalDeviceVulkan12Features &features12 =
            features.get<vk::PhysicalDeviceVulkan12Features>();
        // Features from VK_KHR_ray_tracing_pipeline
        const vk::PhysicalDeviceRayTracingPipelineFeaturesKHR
            &ray_tracing_features =
                features.get<vk::PhysicalDeviceRayTracingPipelineFeaturesKHR>();
        // Features from VK_KHR_acceleration_structure
        const vk::PhysicalDeviceAccelerationStructureFeaturesKHR
            &acceleration_structure_features =
                features
                    .get<vk::PhysicalDeviceAccelerationStructureFeaturesKHR>();

        if (surface_supported && features13.dynamicRendering &&
            features13.synchronization2 && features12.bufferDeviceAddress &&
            features12.descriptorIndexing &&
            ray_tracing_features.rayTracingPipeline &&
            acceleration_structure_features.accelerationStructure) {
            return physical_device.get();
        }
    }
    throw std::runtime_error("No suitable physical device found");
}
} // namespace kovra
