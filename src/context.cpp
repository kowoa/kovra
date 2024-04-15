
#include "context.hpp"
#include "spdlog/spdlog.h"
#include <memory>

namespace kovra {
std::shared_ptr<PhysicalDevice>
pick_physical_device(
  const std::vector<std::shared_ptr<PhysicalDevice>> &physical_devices,
  const Surface &surface
);

Context::Context(SDL_Window *window, bool enable_multisampling)
  : instance{ std::make_unique<Instance>(window) }
  , surface{ std::make_unique<Surface>(*instance, window) }
  , physical_device{ pick_physical_device(
      instance->enumerate_physical_devices(*surface),
      *surface
    ) }
  , device{ std::make_shared<Device>(*instance, physical_device) }
  , swapchain{
      std::make_unique<Swapchain>(window, *surface, *physical_device, *device)
  }
{
    spdlog::debug("Context::Context()");
}

Context::~Context()
{
    spdlog::debug("Context::~Context()");
    swapchain.reset();
    device.reset();
    physical_device.reset();
    surface.reset();
    instance.reset();
}

void
Context::recreate_swapchain(SDL_Window *window)
{
    device->get().waitIdle();
    swapchain.reset();
    swapchain =
      std::make_unique<Swapchain>(window, *surface, *physical_device, *device);
}

std::shared_ptr<PhysicalDevice>
pick_physical_device(
  const std::vector<std::shared_ptr<PhysicalDevice>> &physical_devices,
  const Surface &surface
)
{
    auto scores = std::vector<int>(physical_devices.size(), 0);

    // Loop through each physical device
    for (size_t i = 0; i < physical_devices.size(); i++) {
        const auto &physical_device = physical_devices[i];

        // Check if the physical device supports the surface
        bool surface_supported = physical_device->get().getSurfaceSupportKHR(
          physical_device->get_present_queue_family().get_index(), surface.get()
        );

        // Check if the physical device supports the required features
        auto features = physical_device->get_supported_features();

        scores[i] += surface_supported ? 1 : 0;
        scores[i] += features.dynamic_rendering ? 1 : 0;
        scores[i] += features.synchronization2 ? 1 : 0;
        scores[i] += features.buffer_device_address ? 1 : 0;
        scores[i] += features.runtime_descriptor_array ? 1 : 0;
        scores[i] += features.ray_tracing_pipeline ? 1 : 0;
        scores[i] += features.acceleration_structure ? 1 : 0;
    }

    // Find the best physical device
    return physical_devices[std::distance(
      scores.begin(), std::max_element(scores.begin(), scores.end())
    )];
}

} // namespace kovra
