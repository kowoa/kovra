#pragma once

#include "context.hpp"
#include <vulkan/vulkan_handles.hpp>

namespace kovra {
class Swapchain {
  public:
    Swapchain(const Context &context, SDL_Window *window);
    ~Swapchain();

    [[nodiscard]] vk::SwapchainKHR get() const { return swapchain.get(); }
    [[nodiscard]] const std::vector<vk::Image> &get_images() const {
        return images;
    }
    [[nodiscard]] const std::vector<vk::UniqueImageView> &get_views() const {
        return views;
    }
    [[nodiscard]] vk::Format get_format() const { return format; }
    [[nodiscard]] vk::Extent2D get_extent() const { return extent; }
    [[nodiscard]] vk::ColorSpaceKHR get_color_space() const {
        return color_space;
    }
    [[nodiscard]] vk::PresentModeKHR get_present_mode() const {
        return present_mode;
    }

  private:
    std::shared_ptr<Device> device;
    vk::UniqueSwapchainKHR swapchain;
    std::vector<vk::Image> images;
    std::vector<vk::UniqueImageView> views;
    vk::Format format;
    vk::Extent2D extent;
    vk::ColorSpaceKHR color_space;
    vk::PresentModeKHR present_mode;
};
} // namespace kovra