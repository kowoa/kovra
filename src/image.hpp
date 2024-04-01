#pragma once

#include "vk_mem_alloc.h"
#include <memory>
#include <vulkan/vulkan.hpp>

namespace kovra {
// Forward declarations
class Device;

struct GpuImageCreateInfo
{
    vk::Format format;
    vk::Extent3D extent;
    vk::ImageUsageFlags usage;
    vk::ImageAspectFlags aspect;
    bool mipmapped;
    std::optional<vk::Sampler> sampler;
};

class GpuImage
{
  public:
    GpuImage(
      const GpuImageCreateInfo &info,
      const vk::Device &device,
      std::shared_ptr<VmaAllocator> allocator
    );
    ~GpuImage();

    GpuImage(const GpuImage &) = delete;
    GpuImage &operator=(const GpuImage &) = delete;
    GpuImage(GpuImage &&) noexcept = delete;
    GpuImage &operator=(GpuImage &&) noexcept = delete;

    // Create a 32-bit shader-readable image from a byte array
    [[nodiscard]] static std::unique_ptr<GpuImage> new_color_image(
      const void *data,
      uint32_t width,
      uint32_t height,
      const Device &device,
      vk::Sampler sampler,
      vk::Format format = vk::Format::eR8G8B8A8Unorm
    );
    // Create an image used for the depth buffer
    [[nodiscard]] static std::unique_ptr<GpuImage> new_depth_image(
      uint32_t width,
      uint32_t height,
      std::optional<vk::Sampler> sampler,
      const Device &device
    );
    // Create an image used by compute shaders
    [[nodiscard]] static std::unique_ptr<GpuImage> new_storage_image(
      uint32_t width,
      uint32_t height,
      std::optional<vk::Sampler> sampler,
      const Device &device
    );

    void transition_layout(
      vk::CommandBuffer cmd,
      vk::ImageLayout old_layout,
      vk::ImageLayout new_layout
    ) noexcept;
    void copy_to(vk::CommandBuffer cmd, const GpuImage &dst) const noexcept;
    void copy_to_vkimage(
      vk::CommandBuffer cmd,
      const vk::Image &dst,
      const vk::Extent2D &dst_extent
    ) const noexcept;

    [[nodiscard]] vk::Image get() const noexcept { return image; }
    [[nodiscard]] vk::ImageView get_view() const noexcept { return view.get(); }
    [[nodiscard]] vk::Format get_format() const noexcept { return format; }
    [[nodiscard]] vk::Extent3D get_extent() const noexcept { return extent; }
    [[nodiscard]] vk::Extent2D get_extent2d() const noexcept
    {
        return vk::Extent2D{ extent.width, extent.height };
    }
    [[nodiscard]] vk::ImageAspectFlags get_aspect() const noexcept
    {
        return aspect;
    }
    [[nodiscard]] vk::Sampler get_sampler() const noexcept
    {
        return sampler.value();
    }

  private:
    std::shared_ptr<VmaAllocator> allocator;
    VkImage image;
    VmaAllocation allocation; // GPU-only memory allocation
    VmaAllocationInfo allocation_info;
    vk::UniqueImageView view;
    vk::Format format;
    vk::Extent3D extent;
    vk::ImageAspectFlags aspect;
    std::optional<vk::Sampler> sampler;

    void upload(const void *data, const Device &device);
};
} // namespace kovra
