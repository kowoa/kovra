#pragma once

#include "context.hpp"
#include <vulkan/vulkan_handles.hpp>

namespace kovra {
class Swapchain {
  public:
    Swapchain(const Context &context, SDL_Window *window);

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
