#pragma once

#include "swapchain.hpp"
#include <memory>

namespace kovra {
class Context
{
  public:
    explicit Context(SDL_Window *window, bool enable_multisampling);
    ~Context();
    Context() = delete;
    Context(const Context &) = delete;
    Context &operator=(const Context &) = delete;
    Context(Context &&) = delete;
    Context &operator=(Context &&) = delete;

    void recreate_swapchain(SDL_Window *window);

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
    [[nodiscard]] const Swapchain &get_swapchain() const
    {
        if (swapchain == nullptr) {
            throw std::runtime_error("Swapchain is null");
        }
        return *swapchain;
    }
    [[nodiscard]] Swapchain &get_swapchain_mut() const
    {
        if (swapchain == nullptr) {
            throw std::runtime_error("Swapchain is null");
        }
        return *swapchain;
    }

  private:
    std::unique_ptr<Instance> instance;
    std::unique_ptr<Surface> surface;
    std::shared_ptr<PhysicalDevice> physical_device;
    std::shared_ptr<Device> device;
    std::unique_ptr<Swapchain> swapchain;
};
} // namespace kovra
