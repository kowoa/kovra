#pragma once

#include "device.hpp"
#include <vulkan/vulkan_handles.hpp>

namespace kovra {
class Swapchain
{
  public:
    Swapchain(
      SDL_Window *window,
      const Surface &surface,
      const PhysicalDevice &physical_device,
      const Device &device
    );
    ~Swapchain();
    Swapchain() = delete;
    Swapchain(const Swapchain &) = delete;
    Swapchain &operator=(const Swapchain &) = delete;
    Swapchain(Swapchain &&) = delete;
    Swapchain &operator=(Swapchain &&) = delete;

    [[nodiscard]] vk::SwapchainKHR get() const { return swapchain.get(); }
    [[nodiscard]] const std::vector<vk::Image> &get_images() const
    {
        return images;
    }
    [[nodiscard]] const std::vector<vk::UniqueImageView> &get_views() const
    {
        return views;
    }
    [[nodiscard]] vk::Format get_format() const { return format; }
    [[nodiscard]] vk::Extent2D get_extent() const { return extent; }
    [[nodiscard]] vk::ColorSpaceKHR get_color_space() const
    {
        return color_space;
    }
    [[nodiscard]] vk::PresentModeKHR get_present_mode() const
    {
        return present_mode;
    }
    [[nodiscard]] const GpuImage &get_depth_image() const
    {
        return *depth_image;
    }

    void request_resize() noexcept { dirty = true; }
    bool is_dirty() const noexcept { return dirty; }

  private:
    vk::UniqueSwapchainKHR swapchain;
    std::vector<vk::Image> images;
    std::vector<vk::UniqueImageView> views;
    vk::Format format;
    vk::Extent2D extent;
    vk::ColorSpaceKHR color_space;
    vk::PresentModeKHR present_mode;

    std::unique_ptr<GpuImage> depth_image;

    bool dirty = false;
};
} // namespace kovra
