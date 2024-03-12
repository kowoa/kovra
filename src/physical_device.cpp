#include "physical_device.hpp"
#include "instance.hpp"
#include "spdlog/spdlog.h"

namespace kovra {
PhysicalDevice::PhysicalDevice(
    vk::PhysicalDevice, const Instance &instance, const Surface &surface) {
    spdlog::info("PhysicalDevice::PhysicalDevice()");
}
} // namespace kovra
