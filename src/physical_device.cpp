#include <iostream>

#include "instance.hpp"
#include "physical_device.hpp"

namespace kovra {
PhysicalDevice::PhysicalDevice(
    vk::PhysicalDevice, const Instance &instance, const Surface &surface) {
    std::cout << "PhysicalDevice::PhysicalDevice()" << std::endl;
}
} // namespace kovra
