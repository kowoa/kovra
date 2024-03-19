#pragma once

#include "vk_mem_alloc.h"
#include <memory>
#include <vulkan/vulkan.hpp>

namespace kovra {
class GpuBuffer {
  public:
    GpuBuffer(
        std::shared_ptr<VmaAllocator> allocator, vk::DeviceSize size,
        vk::BufferUsageFlags buffer_usage, VmaMemoryUsage alloc_usage,
        VmaAllocationCreateFlags alloc_flags);
    ~GpuBuffer();

    void write(const void *data, vk::DeviceSize size);

    [[nodiscard]] vk::Buffer get() const { return buffer; }
    [[nodiscard]] vk::DeviceSize get_size() const {
        return allocation_info.size;
    }

  private:
    std::shared_ptr<VmaAllocator> allocator;
    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo allocation_info;
    bool is_mapped;
};
} // namespace kovra
