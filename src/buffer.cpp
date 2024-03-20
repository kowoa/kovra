#include "buffer.hpp"
#include "spdlog/spdlog.h"

namespace kovra {

GpuBuffer::GpuBuffer(
    std::shared_ptr<VmaAllocator> allocator, vk::DeviceSize size,
    vk::BufferUsageFlags buffer_usage, VmaMemoryUsage alloc_usage,
    VmaAllocationCreateFlags alloc_flags) {
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

    auto result = vmaCreateBuffer(
        *allocator, &buffer_info, &alloc_info, &buffer, &allocation,
        &allocation_info);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create buffer");
    }

    this->allocator = allocator;
}

GpuBuffer::~GpuBuffer() {
    spdlog::debug("GpuBuffer::~GpuBuffer()");
    allocator.reset();
    vmaDestroyBuffer(*allocator, buffer, allocation);
    vmaFreeMemory(*allocator, allocation);
}

void GpuBuffer::write(const void *data, vk::DeviceSize size) {
    if (allocation_info.pMappedData != nullptr) {
        std::memcpy(allocation_info.pMappedData, data, size);
        return;
    }

    void *mapped_data;
    vmaMapMemory(*allocator, allocation, &mapped_data);
    std::memcpy(mapped_data, data, size);
    vmaUnmapMemory(*allocator, allocation);
}

} // namespace kovra
