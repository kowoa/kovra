#include "swapchain.hpp"
#include "image.hpp"
#include "spdlog/spdlog.h"

namespace kovra {
Swapchain::Swapchain(
    SDL_Window *window, const Surface &surface,
    const PhysicalDevice &physical_device, const Device &device) {
    spdlog::debug("Swapchain::Swapchain()");

    // Swapchain extent
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    extent = vk::Extent2D{
        static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

    // Swapchain format and color space
    auto formats = physical_device.get().getSurfaceFormatsKHR(surface.get());
    if (formats.size() == 1 && formats[0].format == vk::Format::eUndefined) {
        format = vk::Format::eB8G8R8A8Unorm;
        color_space = formats[0].colorSpace;
    } else {
        bool found = false;
        for (const auto &f : formats) {
            if (f.format == vk::Format::eB8G8R8A8Unorm &&
                f.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                found = true;
                format = f.format;
                color_space = f.colorSpace;
                break;
            }
        }
        if (!found) {
            format = formats[0].format;
            color_space = formats[0].colorSpace;
        }
    }

    // Swapchain present mode
    auto present_modes =
        physical_device.get().getSurfacePresentModesKHR(surface.get());
    if (std::find(
            present_modes.begin(), present_modes.end(),
            vk::PresentModeKHR::eMailbox) != present_modes.end()) {
        present_mode = vk::PresentModeKHR::eMailbox;
    } else {
        present_mode = vk::PresentModeKHR::eFifo;
    }

    // Swapchain
    auto capabilities =
        physical_device.get().getSurfaceCapabilitiesKHR(surface.get());
    auto image_count = capabilities.minImageCount + 1;
    auto sharing_mode =
        device.get_graphics_family_index() != device.get_present_family_index()
            ? vk::SharingMode::eConcurrent
            : vk::SharingMode::eExclusive;
    auto queue_family_indices = std::array{
        device.get_graphics_family_index(), device.get_present_family_index()};
    swapchain = device.get().createSwapchainKHRUnique(
        vk::SwapchainCreateInfoKHR{}
            .setSurface(surface.get())
            .setMinImageCount(image_count)
            .setImageFormat(format)
            .setImageColorSpace(color_space)
            .setImageExtent(extent)
            .setImageArrayLayers(1)
            .setImageUsage(
                vk::ImageUsageFlagBits::eColorAttachment |
                vk::ImageUsageFlagBits::eTransferDst)
            .setImageSharingMode(sharing_mode)
            .setQueueFamilyIndices(queue_family_indices)
            .setQueueFamilyIndexCount(
                static_cast<uint32_t>(queue_family_indices.size()))
            .setPreTransform(capabilities.currentTransform)
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
            .setPresentMode(present_mode)
            .setClipped(true)
            .setOldSwapchain(nullptr));

    // Swapchain images and image views
    images = device.get().getSwapchainImagesKHR(swapchain.get());
    views.reserve(images.size());
    for (const auto &image : images) {
        views.emplace_back(device.get().createImageViewUnique(
            vk::ImageViewCreateInfo{}
                .setImage(image)
                .setViewType(vk::ImageViewType::e2D)
                .setFormat(format)
                .setComponents(vk::ComponentMapping{})
                .setSubresourceRange(
                    vk::ImageSubresourceRange{}
                        .setAspectMask(vk::ImageAspectFlagBits::eColor)
                        .setBaseMipLevel(0)
                        .setLevelCount(1)
                        .setBaseArrayLayer(0)
                        .setLayerCount(1))));
    }

    // Depth image
    depth_image =
        device.create_depth_image(extent.width, extent.height, std::nullopt);
}

Swapchain::~Swapchain() { spdlog::debug("Swapchain::~Swapchain()"); }
} // namespace kovra
