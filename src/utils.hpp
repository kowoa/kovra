#pragma once

#include <cstdint>
#include <vulkan/vulkan.hpp>

namespace kovra {
namespace utils {
inline uint32_t aligned_size(uint32_t size, uint32_t alignment) {
    if (alignment == 0) {
        return size;
    }
    return (size + alignment - 1) & ~(alignment - 1);
}

void transition_image_layout(
    vk::CommandBuffer cmd, vk::Image image, vk::ImageAspectFlags aspect,
    vk::ImageLayout old_layout, vk::ImageLayout new_layout);
void copy_image_to_image(
    vk::CommandBuffer cmd, vk::Image src, vk::Image dst, vk::Extent2D src_size,
    vk::Extent2D dst_size);
} // namespace utils
} // namespace kovra
