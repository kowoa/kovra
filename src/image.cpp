#include "image.hpp"
#include "device.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"

namespace kovra {
GpuImage::GpuImage(
    const GpuImageDescriptor &desc, const vk::Device &device,
    std::shared_ptr<VmaAllocator> allocator)
    : allocator{allocator}, format{desc.format}, extent{desc.extent},
      aspect{desc.aspect}, sampler{desc.sampler} {
    VkImageCreateInfo image_ci{};
    image_ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = static_cast<VkFormat>(desc.format);
    image_ci.extent = desc.extent;
    if (desc.mipmapped) {
        image_ci.mipLevels =
            static_cast<uint32_t>(std::floor(
                std::log2(std::max(desc.extent.width, desc.extent.height)))) +
            1;
    } else {
        image_ci.mipLevels = 1;
    }
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = static_cast<VkImageUsageFlags>(desc.usage);

    // Always allocate images on dedicated GPU memory
    VmaAllocationCreateInfo alloc_ci{};
    alloc_ci.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    alloc_ci.requiredFlags =
        VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // Allocate and create image
    if (VkResult result = vmaCreateImage(
            *this->allocator, &image_ci, &alloc_ci, &image, &allocation,
            &allocation_info);
        result != VK_SUCCESS) {
        switch (result) {
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            spdlog::error("Failed to allocate image: out of device memory");
            break;
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            spdlog::error("Failed to allocate image: out of host memory");
            break;
        case VK_ERROR_INVALID_EXTERNAL_HANDLE:
            spdlog::error("Failed to allocate image: invalid external handle");
            break;
        case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
            spdlog::error(
                "Failed to allocate image: invalid opaque capture address");
            break;
        case VK_ERROR_FORMAT_NOT_SUPPORTED:
            spdlog::error("Failed to allocate image: format not supported");
            break;
        case VK_ERROR_FRAGMENTED_POOL:
            spdlog::error("Failed to allocate image: fragmented pool");
            break;
        case VK_ERROR_INITIALIZATION_FAILED:
            spdlog::error("Failed to allocate image: initialization failed");
            break;
        case VK_ERROR_DEVICE_LOST:
            spdlog::error("Failed to allocate image: device lost");
            break;
        case VK_ERROR_SURFACE_LOST_KHR:
            spdlog::error("Failed to allocate image: surface lost");
            break;
        default:
            spdlog::error("Failed to allocate image: unknown error");
        }

        throw std::runtime_error("Failed to create image");
    }

    // Create image view
    view = device.createImageViewUnique(
        vk::ImageViewCreateInfo{}
            .setImage(image)
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(desc.format)
            .setSubresourceRange(
                vk::ImageSubresourceRange{}
                    .setAspectMask(desc.aspect)
                    .setBaseMipLevel(0)
                    .setLevelCount(desc.mipmapped ? image_ci.mipLevels : 1)
                    .setBaseArrayLayer(0)
                    .setLayerCount(1)));
}

GpuImage::~GpuImage() {
    view.reset();
    vmaDestroyImage(*allocator, image, allocation);
}

// Create a 32-bit shader-readable image from a byte array
std::unique_ptr<GpuImage> GpuImage::new_color_image(
    const void *data, uint32_t width, uint32_t height,
    std::optional<vk::Sampler> sampler, const Device &device) {
    auto desc = GpuImageDescriptor{
        .format = vk::Format::eR8G8B8A8Unorm,
        .extent = vk::Extent3D{width, height, 1},
        .usage = vk::ImageUsageFlagBits::eSampled |
                 vk::ImageUsageFlagBits::eTransferDst,
        .aspect = vk::ImageAspectFlagBits::eColor,
        .mipmapped = false,
        .sampler = sampler};
    auto image = std::make_unique<GpuImage>(
        desc, device.get(), device.get_allocator_owned());
    image->upload(data, device);
    return image;
}
// Create an image used for the depth buffer
std::unique_ptr<GpuImage> GpuImage::new_depth_image(
    uint32_t width, uint32_t height, std::optional<vk::Sampler> sampler,
    const Device &device) {
    auto desc = GpuImageDescriptor{
        .format = vk::Format::eD32Sfloat,
        .extent = vk::Extent3D{width, height, 1},
        .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
        .aspect = vk::ImageAspectFlagBits::eDepth,
        .mipmapped = false,
        .sampler = sampler};
    return std::make_unique<GpuImage>(
        desc, device.get(), device.get_allocator_owned());
}
// Create an image used by compute shaders
std::unique_ptr<GpuImage> GpuImage::new_storage_image(
    uint32_t width, uint32_t height, std::optional<vk::Sampler> sampler,
    const Device &device) {
    auto desc = GpuImageDescriptor{
        .format = vk::Format::eR16G16B16A16Sfloat,
        .extent = vk::Extent3D{width, height, 1},
        .usage = vk::ImageUsageFlagBits::eStorage |
                 vk::ImageUsageFlagBits::eTransferSrc |
                 vk::ImageUsageFlagBits::eTransferDst,
        .aspect = vk::ImageAspectFlagBits::eColor,
        .mipmapped = false,
        .sampler = sampler};
    return std::make_unique<GpuImage>(
        desc, device.get(), device.get_allocator_owned());
}

void GpuImage::transition_layout(
    vk::CommandBuffer cmd, vk::ImageLayout old_layout,
    vk::ImageLayout new_layout) noexcept {
    utils::transition_image_layout(cmd, image, aspect, old_layout, new_layout);
}
void GpuImage::copy_to(
    vk::CommandBuffer cmd, const GpuImage &dst) const noexcept {
    utils::copy_image_to_image(
        cmd, image, dst.image, vk::Extent2D{extent.width, extent.height},
        vk::Extent2D{dst.extent.width, dst.extent.height});
}
void GpuImage::copy_to_vkimage(
    vk::CommandBuffer cmd, const vk::Image &dst,
    const vk::Extent2D &dst_extent) const noexcept {
    utils::copy_image_to_image(
        cmd, image, dst, vk::Extent2D{extent.width, extent.height}, dst_extent);
}
void GpuImage::upload(const void *data, const Device &device) {
    vk::DeviceSize data_size = extent.depth * extent.width * extent.height * 4;
    auto staging_buffer = device.create_buffer(
        data_size, vk::BufferUsageFlagBits::eTransferSrc,
        VMA_MEMORY_USAGE_CPU_TO_GPU, VMA_ALLOCATION_CREATE_MAPPED_BIT);
    staging_buffer->write(data, data_size);

    device.immediate_submit([&](vk::CommandBuffer cmd) {
        transition_layout(
            cmd, vk::ImageLayout::eUndefined,
            vk::ImageLayout::eTransferDstOptimal);

        auto region =
            vk::BufferImageCopy{}
                .setBufferOffset(0)
                .setBufferRowLength(0)
                .setBufferImageHeight(0)
                .setImageSubresource(
                    vk::ImageSubresourceLayers{}
                        .setAspectMask(vk::ImageAspectFlagBits::eColor)
                        .setMipLevel(0)
                        .setBaseArrayLayer(0)
                        .setLayerCount(1))
                .setImageExtent(extent);
        cmd.copyBufferToImage(
            staging_buffer->get(), image, vk::ImageLayout::eTransferDstOptimal,
            1, &region);

        transition_layout(
            cmd, vk::ImageLayout::eTransferDstOptimal,
            vk::ImageLayout::eShaderReadOnlyOptimal);
    });
}
} // namespace kovra
