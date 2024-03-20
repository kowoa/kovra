#include "image.hpp"
#include "device.hpp"
#include "utils.hpp"

namespace kovra {
GpuImage::GpuImage(
    const GpuImageDescriptor &desc, const vk::Device &device,
    std::shared_ptr<VmaAllocator> allocator) {
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
    if (vmaCreateImage(
            *allocator, &image_ci, &alloc_ci, &image, &allocation,
            &allocation_info) != VK_SUCCESS) {
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
    vmaDestroyImage(*allocator, image, allocation);
    vmaFreeMemory(*allocator, allocation);
}

// Create a 32-bit shader-readable image from a byte array
std::unique_ptr<GpuImage> GpuImage::new_color_image(
    const void *data, uint32_t width, uint32_t height, const Device &device,
    std::shared_ptr<VmaAllocator> allocator) {
    auto desc = GpuImageDescriptor{
        .format = vk::Format::eR8G8B8A8Unorm,
        .extent = vk::Extent3D{width, height, 1},
        .usage = vk::ImageUsageFlagBits::eSampled |
                 vk::ImageUsageFlagBits::eTransferDst,
        .aspect = vk::ImageAspectFlagBits::eColor,
        .mipmapped = false};
    auto image = std::make_unique<GpuImage>(desc, device.get(), allocator);
    image->upload(data, device);
    return image;
}
// Create an image used for the depth buffer
std::unique_ptr<GpuImage> GpuImage::new_depth_image(
    uint32_t width, uint32_t height, const Device &device,
    std::shared_ptr<VmaAllocator> allocator) {
    auto desc = GpuImageDescriptor{
        .format = vk::Format::eD32Sfloat,
        .extent = vk::Extent3D{width, height, 1},
        .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
        .aspect = vk::ImageAspectFlagBits::eDepth,
        .mipmapped = false};
    return std::make_unique<GpuImage>(desc, device.get(), allocator);
}
// Create an image used by compute shaders
std::unique_ptr<GpuImage> GpuImage::new_storage_image(
    uint32_t width, uint32_t height, const Device &device,
    std::shared_ptr<VmaAllocator> allocator) {
    auto desc = GpuImageDescriptor{
        .format = vk::Format::eR16G16B16A16Sfloat,
        .extent = vk::Extent3D{width, height, 1},
        .usage = vk::ImageUsageFlagBits::eStorage |
                 vk::ImageUsageFlagBits::eTransferSrc |
                 vk::ImageUsageFlagBits::eTransferDst,
        .aspect = vk::ImageAspectFlagBits::eColor,
        .mipmapped = false};
    return std::make_unique<GpuImage>(desc, device.get(), allocator);
}

void GpuImage::transition_layout(
    vk::ImageLayout old_layout, vk::ImageLayout new_layout,
    vk::CommandBuffer cmd) noexcept {
    utils::transition_image_layout(cmd, image, aspect, old_layout, new_layout);
}
void GpuImage::copy_to(
    const GpuImage &dst, vk::CommandBuffer cmd) const noexcept {
    utils::copy_image_to_image(
        cmd, image, dst.image, vk::Extent2D{extent.width, extent.height},
        vk::Extent2D{dst.extent.width, dst.extent.height});
}
void upload(const void *data, const Device &device);
} // namespace kovra
