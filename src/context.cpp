
#include "context.hpp"
#include "spdlog/spdlog.h"
#include <memory>

namespace kovra {
std::shared_ptr<PhysicalDevice> pick_physical_device(
    const std::vector<std::shared_ptr<PhysicalDevice>> &physical_devices,
    const Surface &surface);

Context::Context(SDL_Window *window)
    : instance{std::make_unique<Instance>(window)},
      surface{std::make_unique<Surface>(*instance, window)},
      physical_device{pick_physical_device(
          instance->enumerate_physical_devices(*surface), *surface)},
      device{std::make_shared<Device>(*instance, physical_device)},
      swapchain{std::make_shared<Swapchain>(
          window, *surface, *physical_device, *device)} {
    spdlog::debug("Context::Context()");
}

Context::~Context() {
    spdlog::debug("Context::~Context()");
    swapchain.reset();
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

} // namespace kovra
