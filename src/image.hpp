#pragma

#include "instance.hpp"
#include "vk_mem_alloc.h"

namespace kovra {
// Forward declarations
class Device;

struct GpuImageDescriptor {
    vk::Format format;
    vk::Extent3D extent;
    vk::ImageUsageFlags usage;
    vk::ImageAspectFlags aspect;
    bool mipmapped;
};

class GpuImage {
  public:
    GpuImage(
        const GpuImageDescriptor &desc, const vk::Device &device,
        std::shared_ptr<VmaAllocator> allocator);
    ~GpuImage();

    GpuImage(const GpuImage &) = delete;

    // Create a 32-bit shader-readable image from a byte array
    static std::unique_ptr<GpuImage> new_color_image(
        const void *data, uint32_t width, uint32_t height, const Device &device,
        std::shared_ptr<VmaAllocator> allocator);
    // Create an image used for the depth buffer
    static std::unique_ptr<GpuImage> new_depth_image(
        uint32_t width, uint32_t height, const Device &device,
        std::shared_ptr<VmaAllocator> allocator);
    // Create an image used by compute shaders
    static std::unique_ptr<GpuImage> new_storage_image(
        uint32_t width, uint32_t height, const Device &device,
        std::shared_ptr<VmaAllocator> allocator);

    void transition_layout(
        vk::ImageLayout old_layout, vk::ImageLayout new_layout,
        vk::CommandBuffer cmd) noexcept;
    void copy_to(const GpuImage &dst, vk::CommandBuffer cmd) const noexcept;

  private:
    std::shared_ptr<VmaAllocator> allocator;
    VkImage image;
    vk::UniqueImageView view;
    vk::Format format;
    vk::Extent3D extent;
    vk::ImageAspectFlags aspect;
    VmaAllocation allocation; // GPU-only memory allocation
    VmaAllocationInfo allocation_info;

    void upload(const void *data, const Device &device);
};
} // namespace kovra
