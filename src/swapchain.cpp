#include "swapchain.hpp"
#include "spdlog/spdlog.h"

namespace kovra {
Swapchain::Swapchain(const Context &context, SDL_Window *window) {
    spdlog::debug("Swapchain::Swapchain()");

    device = context.device;

    // Swapchain extent
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    extent = vk::Extent2D{
        static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

    // Swapchain format and color space
    auto formats = context.physical_device.get()->get().getSurfaceFormatsKHR(
        context.surface.get()->get());
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
        context.physical_device.get()->get().getSurfacePresentModesKHR(
            context.surface.get()->get());
    if (std::find(
            present_modes.begin(), present_modes.end(),
            vk::PresentModeKHR::eMailbox) != present_modes.end()) {
        present_mode = vk::PresentModeKHR::eMailbox;
    } else {
        present_mode = vk::PresentModeKHR::eFifo;
    }

    // Swapchain
    auto capabilities =
        context.physical_device.get()->get().getSurfaceCapabilitiesKHR(
            context.surface.get()->get());
    auto image_count = capabilities.minImageCount + 1;
    auto sharing_mode = context.graphics_queue_family.get_index() !=
                                context.present_queue_family.get_index()
                            ? vk::SharingMode::eConcurrent
                            : vk::SharingMode::eExclusive;
    auto queue_family_indices = std::array{
        context.graphics_queue_family.get_index(),
        context.present_queue_family.get_index()};
    swapchain = context.device.get()->get().createSwapchainKHRUnique(
        vk::SwapchainCreateInfoKHR{}
            .setSurface(context.surface.get()->get())
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
    images = context.device.get()->get().getSwapchainImagesKHR(swapchain.get());
    views = std::vector<vk::UniqueImageView>(images.size());
    for (const auto &image : images) {
        views.emplace_back(context.device.get()->get().createImageViewUnique(
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
};
} // namespace kovra
