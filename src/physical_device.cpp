#include "physical_device.hpp"
#include "queue.hpp"
#include "spdlog/spdlog.h"

namespace kovra {
PhysicalDevice::PhysicalDevice(
  const vk::PhysicalDevice physical_device,
  const Surface &surface
)
  : supported_surface_formats{ physical_device.getSurfaceFormatsKHR(surface.get(
    )) }
  , supported_present_modes{ physical_device.getSurfacePresentModesKHR(
      surface.get()
    ) }
  , supported_features{ physical_device }
{
    spdlog::debug("PhysicalDevice::PhysicalDevice()");

    this->physical_device = physical_device;

    const auto props = physical_device.getProperties();
    name = std::string{ props.deviceName };
    type = props.deviceType;
    limits = props.limits;

    auto queue_family_props = physical_device.getQueueFamilyProperties();
    for (size_t i = 0; i < queue_family_props.size(); i++) {
        bool present_support =
          physical_device.getSurfaceSupportKHR(i, surface.get());
        queue_families.emplace_back(
          static_cast<uint32_t>(i), queue_family_props[i], present_support
        );
    }

    for (auto extension_props =
           physical_device.enumerateDeviceExtensionProperties();
         const auto &extension : extension_props) {
        supported_extensions.emplace_back(extension.extensionName);
    }
}

PhysicalDevice::~PhysicalDevice()
{
    spdlog::debug("PhysicalDevice::~PhysicalDevice()");
}

[[nodiscard]] QueueFamily
PhysicalDevice::get_graphics_queue_family() const
{
    for (const auto &queue_family : queue_families) {
        if (queue_family.has_graphics_support()) {
            return queue_family;
        }
    }
    throw std::runtime_error("No suitable graphics queue family found");
}

[[nodiscard]] QueueFamily
PhysicalDevice::get_present_queue_family() const
{
    for (const auto &queue_family : queue_families) {
        if (queue_family.has_present_support()) {
            return queue_family;
        }
    }
    throw std::runtime_error("No suitable present queue family found");
}

[[nodiscard]] QueueFamily
PhysicalDevice::get_transfer_queue_family() const
{
    for (const auto &queue_family : queue_families) {
        if (queue_family.has_transfer_support()) {
            return queue_family;
        }
    }
    throw std::runtime_error("No suitable transfer queue family found");
}

[[nodiscard]] QueueFamily
PhysicalDevice::get_compute_queue_family() const
{
    for (const auto &queue_family : queue_families) {
        if (queue_family.has_compute_support()) {
            return queue_family;
        }
    }
    throw std::runtime_error("No suitable compute queue family found");
}

[[nodiscard]] bool
PhysicalDevice::supports_extensions(const std::vector<std::string> &extensions
) const noexcept
{
    // Check if each extension in the argument exists in supported_extensions
    return std::ranges::all_of(extensions, [this](const auto &ext) {
        return std::ranges::find(supported_extensions, ext) !=
               supported_extensions.end();
    });
}
} // namespace kovra
