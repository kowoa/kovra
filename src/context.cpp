#define VMA_IMPLEMENTATION

#include "context.hpp"
#include "spdlog/spdlog.h"
#include <memory>

namespace kovra {
std::shared_ptr<PhysicalDevice> pick_physical_device(
    const std::vector<std::shared_ptr<PhysicalDevice>> &physical_devices,
    const Surface &surface);
VmaAllocator create_allocator(
    const Instance &instance, const PhysicalDevice &physical_device,
    const Device &device);

Context::Context(SDL_Window *window)
    : instance{std::make_unique<Instance>(window)},
      surface{std::make_unique<Surface>(*instance, window)},
      physical_device{pick_physical_device(
          instance->enumerate_physical_devices(*surface), *surface)},
      device{std::make_shared<Device>(physical_device)},
      graphics_queue{
          device->get().getQueue(
              physical_device->get_graphics_queue_family().get_index(), 0),
          device},
      present_queue{
          device->get().getQueue(
              physical_device->get_graphics_queue_family().get_index(), 0),
          device},
      graphics_queue_family{physical_device->get_graphics_queue_family()},
      present_queue_family{physical_device->get_present_queue_family()},
      allocator{std::make_unique<VmaAllocator>(
          create_allocator(*instance, *physical_device, *device))},
      command_pool{
          device->get().createCommandPoolUnique(vk::CommandPoolCreateInfo{
              vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
              graphics_queue_family.get_index()})} {
    spdlog::debug("Context::Context()");
}

Context::~Context() {
    spdlog::debug("Context::~Context()");
    vmaDestroyAllocator(*allocator);
    allocator.reset();
    device.reset();
    physical_device.reset();
    surface.reset();
    instance.reset();
}

std::shared_ptr<PhysicalDevice> pick_physical_device(
    const std::vector<std::shared_ptr<PhysicalDevice>> &physical_devices,
    const Surface &surface) {
    // Loop through each physical device
    for (const auto &physical_device : physical_devices) {
        // Check if the physical device supports the surface
        bool surface_supported = physical_device->get().getSurfaceSupportKHR(
            physical_device->get_present_queue_family().get_index(),
            surface.get());

        // Check if the physical device supports the required features
        auto features = physical_device->get_supported_features();
        if (surface_supported && features.dynamic_rendering &&
            features.synchronization2 && features.buffer_device_address &&
            features.runtime_descriptor_array &&
            features.ray_tracing_pipeline && features.acceleration_structure) {
            return physical_device;
        }
    }
    throw std::runtime_error("No suitable physical device found");
}

VmaAllocator create_allocator(
    const Instance &instance, const PhysicalDevice &physical_device,
    const Device &device) {
    VmaAllocatorCreateInfo allocator_ci{};
    allocator_ci.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    allocator_ci.vulkanApiVersion = VK_API_VERSION_1_3;
    allocator_ci.physicalDevice = physical_device.get();
    allocator_ci.device = device.get();
    allocator_ci.instance = instance.get();

    VmaAllocator allocator;
    vmaCreateAllocator(&allocator_ci, &allocator);
    return allocator;
}

} // namespace kovra
