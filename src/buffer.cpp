#include "buffer.hpp"
#include "spdlog/spdlog.h"

namespace kovra {

GpuBuffer::GpuBuffer(
  std::shared_ptr<VmaAllocator> allocator,
  vk::DeviceSize size,
  vk::BufferUsageFlags buffer_usage,
  VmaMemoryUsage alloc_usage,
  VmaAllocationCreateFlags alloc_flags
)
{
    spdlog::debug("GpuBuffer::GpuBuffer()");

    buffer_size = size;

    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = static_cast<VkBufferUsageFlags>(buffer_usage);
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = alloc_usage;
    alloc_info.flags = alloc_flags;

    if (auto result = vmaCreateBuffer(
          *allocator,
          &buffer_info,
          &alloc_info,
          &buffer,
          &allocation,
          &allocation_info
        );
        result != VK_SUCCESS) {
        switch (result) {
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                spdlog::error("Failed to allocate buffer: out of device memory"
                );
                break;
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                spdlog::error("Failed to allocate buffer: out of host memory");
                break;
            case VK_ERROR_INVALID_EXTERNAL_HANDLE:
                spdlog::error(
                  "Failed to allocate buffer: invalid external handle"
                );
                break;
            case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
                spdlog::error(
                  "Failed to allocate buffer: invalid opaque capture address"
                );
                break;
            case VK_ERROR_FORMAT_NOT_SUPPORTED:
                spdlog::error("Failed to allocate buffer: format not supported"
                );
                break;
            case VK_ERROR_FRAGMENTED_POOL:
                spdlog::error("Failed to allocate buffer: fragmented pool");
                break;
            case VK_ERROR_INITIALIZATION_FAILED:
                spdlog::error("Failed to allocate buffer: initialization failed"
                );
                break;
            default:
                spdlog::error("Failed to allocate buffer: unknown error");
        }
        throw std::runtime_error("Failed to create buffer");
    }

    this->allocator = allocator;
}

GpuBuffer::~GpuBuffer()
{
    spdlog::debug("GpuBuffer::~GpuBuffer()");
    vmaDestroyBuffer(*allocator, buffer, allocation);
    allocator.reset();
}

void
GpuBuffer::write(const void *data, size_t size, size_t offset)
{
    if (allocation_info.pMappedData != nullptr) {
        std::memcpy((char *)allocation_info.pMappedData + offset, data, size);
        return;
    }

    void *mapped_data;
    vmaMapMemory(*allocator, allocation, &mapped_data);
    std::memcpy((char *)mapped_data + offset, data, size);
    vmaUnmapMemory(*allocator, allocation);
}

} // namespace kovra
