#pragma once

#include "physical_device.hpp"

namespace kovra {
class Device {
  public:
    Device(std::shared_ptr<PhysicalDevice> physical_device);
    ~Device();

    [[nodiscard]] const vk::Device &get() const noexcept {
        return device.get();
    }

  private:
    vk::UniqueDevice device;
    std::shared_ptr<PhysicalDevice> physical_device;
};
} // namespace kovra
