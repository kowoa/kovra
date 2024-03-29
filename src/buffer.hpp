#pragma once

#include "vk_mem_alloc.h"
#include <memory>
#include <vulkan/vulkan.hpp>

namespace kovra {
class GpuBuffer
{
  public:
    GpuBuffer(
      std::shared_ptr<VmaAllocator> allocator,
      vk::DeviceSize size,
      vk::BufferUsageFlags buffer_usage,
      VmaMemoryUsage alloc_usage,
      VmaAllocationCreateFlags alloc_flags
    );
    ~GpuBuffer();
    GpuBuffer(const GpuBuffer &) = delete;

    void write(const void *data, size_t size, size_t offset = 0);

    [[nodiscard]] vk::Buffer get() const { return buffer; }
    [[nodiscard]] vk::DeviceSize get_size() const { return buffer_size; }

  private:
    std::shared_ptr<VmaAllocator> allocator;
    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo allocation_info;
    uint32_t buffer_size;
};
} // namespace kovra
