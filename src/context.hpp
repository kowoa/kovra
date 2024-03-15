#pragma once

#include "device.hpp"
#include <memory>

namespace kovra {
class Context {
  public:
    explicit Context(SDL_Window *window);
    ~Context();

    [[nodiscard]] const vk::Instance &get_instance() const noexcept {
        return instance->get();
    }
    [[nodiscard]] const vk::SurfaceKHR &get_surface() const noexcept {
        return surface->get();
    }
    [[nodiscard]] const vk::PhysicalDevice &
    get_physical_device() const noexcept {
        return physical_device->get();
    }
    [[nodiscard]] const vk::Device &get_device() const noexcept {
        return device->get();
    }
    [[nodiscard]] const std::shared_ptr<Device> &
    get_device_owned() const noexcept {
        return device;
    }

  private:
    std::unique_ptr<Instance> instance;
    std::unique_ptr<Surface> surface;
    std::shared_ptr<PhysicalDevice>
        physical_device; // Instance also has ownership of all physical devices
    std::shared_ptr<Device> device;
};
} // namespace kovra
