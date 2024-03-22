#include "device.hpp"
#include "image.hpp"
#include "queue.hpp"
#include "spdlog/spdlog.h"
#include <set>

namespace kovra {
std::vector<const char *> get_required_device_extensions();
VmaAllocator create_allocator(
    const Instance &instance, const PhysicalDevice &physical_device,
    const vk::Device &device);

Device::Device(
    const Instance &instance, std::shared_ptr<PhysicalDevice> physical_device)
    : physical_device{physical_device} {

    spdlog::debug("Device::Device()");

    auto queue_families = physical_device->get_queue_families();
    queue_families.erase(
        std::unique(
            queue_families.begin(), queue_families.end(),
            [](const QueueFamily &a, const QueueFamily &b) { return a == b; }),
        queue_families.end());

    std::set<uint32_t> unique_queue_family_indices{
        physical_device->get_graphics_queue_family().get_index(),
        physical_device->get_present_queue_family().get_index()};

    auto queue_priorities = std::array{1.0f};
    auto queue_cis = std::vector<vk::DeviceQueueCreateInfo>();
    queue_cis.reserve(physical_device->get_queue_families().size());
    for (const auto &index : unique_queue_family_indices) {
        queue_cis.emplace_back(vk::DeviceQueueCreateInfo{
            vk::DeviceQueueCreateFlags(), index, queue_priorities.size(),
            queue_priorities.data()});
    }

    std::vector<const char *> req_device_exts =
        get_required_device_extensions();

    auto device_features = physical_device->get_supported_features();
    auto ray_tracing_features =
        vk::PhysicalDeviceRayTracingPipelineFeaturesKHR{}.setRayTracingPipeline(
            device_features.ray_tracing_pipeline);
    auto acceleration_struct_features =
        vk::PhysicalDeviceAccelerationStructureFeaturesKHR{}
            .setAccelerationStructure(device_features.acceleration_structure)
            .setPNext(&ray_tracing_features);
    auto vulkan_12_features =
        vk::PhysicalDeviceVulkan12Features{}
            .setRuntimeDescriptorArray(device_features.runtime_descriptor_array)
            .setBufferDeviceAddress(device_features.buffer_device_address)
            .setPNext(&acceleration_struct_features);
    auto vulkan_13_features =
        vk::PhysicalDeviceVulkan13Features{}
            .setDynamicRendering(device_features.dynamic_rendering)
            .setSynchronization2(device_features.synchronization2)
            .setPNext(&vulkan_12_features);
    auto features = vk::PhysicalDeviceFeatures2{}.setPNext(&vulkan_13_features);

    device = physical_device->get().createDeviceUnique(
        vk::DeviceCreateInfo{}
            .setQueueCreateInfos(queue_cis)
            .setPEnabledExtensionNames(req_device_exts)
            .setPNext(&features));
    VULKAN_HPP_DEFAULT_DISPATCHER.init(device.get());

    graphics_queue = std::make_unique<Queue>(
        physical_device->get_graphics_queue_family(), device.get());
    present_queue = std::make_unique<Queue>(
        physical_device->get_present_queue_family(), device.get());
    transfer_queue = std::make_unique<Queue>(
        physical_device->get_transfer_queue_family(), device.get());
    compute_queue = std::make_unique<Queue>(
        physical_device->get_compute_queue_family(), device.get());

    allocator = std::make_shared<VmaAllocator>(
        create_allocator(instance, *physical_device, *device));
    command_pool =
        device.get().createCommandPoolUnique(vk::CommandPoolCreateInfo{
            vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            graphics_queue->get_family_index()});
    transfer_context =
        std::make_unique<TransferContext>(*transfer_queue, device.get());
}

Device::~Device() {
    spdlog::debug("Device::~Device()");
    device.get().waitIdle();
    transfer_context.reset();
    command_pool.reset();
    vmaDestroyAllocator(*allocator);
    allocator.reset();
    compute_queue.reset();
    transfer_queue.reset();
    present_queue.reset();
    graphics_queue.reset();
    device.reset();
    physical_device.reset();
}

[[nodiscard]] CommandEncoder Device::create_command_encoder() const {
    return CommandEncoder{*this};
}
[[nodiscard]] std::unique_ptr<GpuBuffer> Device::create_buffer(
    vk::DeviceSize size, vk::BufferUsageFlags buffer_usage,
    VmaMemoryUsage alloc_usage, VmaAllocationCreateFlags alloc_flags) const {
    return std::make_unique<GpuBuffer>(
        allocator, size, buffer_usage, alloc_usage, alloc_flags);
}

DeviceFeatures::DeviceFeatures(const vk::PhysicalDevice &physical_device) {
    auto features = physical_device.getFeatures2<
        vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan12Features,
        vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceRayTracingPipelineFeaturesKHR,
        vk::PhysicalDeviceAccelerationStructureFeaturesKHR>();

    const auto &features12 = features.get<vk::PhysicalDeviceVulkan12Features>();
    const auto &features13 = features.get<vk::PhysicalDeviceVulkan13Features>();
    const auto &ray_tracing_features =
        features.get<vk::PhysicalDeviceRayTracingPipelineFeaturesKHR>();
    const auto &acceleration_structure_features =
        features.get<vk::PhysicalDeviceAccelerationStructureFeaturesKHR>();

    dynamic_rendering = features13.dynamicRendering;
    synchronization2 = features13.synchronization2;
    runtime_descriptor_array = features12.runtimeDescriptorArray;
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

[[nodiscard]] std::unique_ptr<GpuImage> Device::create_color_image(
    const void *data, uint32_t width, uint32_t height,
    std::optional<vk::Sampler> sampler) const {
    return GpuImage::new_color_image(data, width, height, sampler, *this);
}
[[nodiscard]] std::unique_ptr<GpuImage> Device::create_depth_image(
    uint32_t width, uint32_t height, std::optional<vk::Sampler> sampler) const {
    return GpuImage::new_depth_image(width, height, sampler, *this);
}
[[nodiscard]] std::unique_ptr<GpuImage> Device::create_storage_image(
    uint32_t width, uint32_t height, std::optional<vk::Sampler> sampler) const {
    return GpuImage::new_storage_image(width, height, sampler, *this);
}
void Device::immediate_submit(
    std::function<void(vk::CommandBuffer)> &&function) const {
    transfer_context->immediate_submit(std::move(function), device.get());
}

std::vector<const char *> get_required_device_extensions() {
    return {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,

        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
        VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
        VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME,
        VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,

        VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,

        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
        VK_KHR_SPIRV_1_4_EXTENSION_NAME,
        VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
    };
}

VmaAllocator create_allocator(
    const Instance &instance, const PhysicalDevice &physical_device,
    const vk::Device &device) {
    VmaAllocatorCreateInfo allocator_ci{};
    allocator_ci.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    allocator_ci.vulkanApiVersion = VK_API_VERSION_1_3;
    allocator_ci.physicalDevice = physical_device.get();
    allocator_ci.device = device;
    allocator_ci.instance = instance.get();

    VmaAllocator allocator;
    if (vmaCreateAllocator(&allocator_ci, &allocator) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create VMA allocator");
    }
    return allocator;
}

} // namespace kovra
