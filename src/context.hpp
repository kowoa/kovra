#pragma once

#include "swapchain.hpp"
#include <memory>

namespace kovra {
class Context
{
  public:
    explicit Context(SDL_Window *window);
    ~Context();

    [[nodiscard]] const vk::Instance &get_instance() const noexcept
    {
        return instance->get();
    }
    [[nodiscard]] const vk::SurfaceKHR &get_surface() const noexcept
    {
        return surface->get();
    }
    [[nodiscard]] const vk::PhysicalDevice &get_physical_device() const noexcept
    {
        return physical_device->get();
    }
    [[nodiscard]] const Device &get_device() const noexcept { return *device; }
    [[nodiscard]] const std::shared_ptr<Device> &get_device_owned(
    ) const noexcept
    {
        return device;
    }
    [[nodiscard]] const Swapchain &get_swapchain() const noexcept
    {
        return *swapchain;
    }
    [[nodiscard]] const std::shared_ptr<Swapchain> &get_swapchain_owned(
    ) const noexcept
    {
        return swapchain;
    }

  private:
    std::unique_ptr<Instance> instance;
    std::unique_ptr<Surface> surface;
    std::shared_ptr<PhysicalDevice> physical_device;
    std::shared_ptr<Device> device;
    std::shared_ptr<Swapchain> swapchain;
};
} // namespace kovra
