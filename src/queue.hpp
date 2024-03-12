#pragma once

#include "device.hpp"
#include <cstdint>
#include <vulkan/vulkan.hpp>

namespace kovra {
class QueueFamily {
  public:
    QueueFamily(
        uint32_t index, vk::QueueFamilyProperties properties,
        bool present_support)
        : properties{properties}, index{index},
          present_support{present_support} {}

    [[nodiscard]] const vk::QueueFamilyProperties &
    get_properties() const noexcept {
        return properties;
    }
    [[nodiscard]] uint32_t get_index() const noexcept { return index; }

    [[nodiscard]] bool has_present_support() const noexcept {
        return present_support;
    }
    [[nodiscard]] bool has_graphics_support() const noexcept {
        return static_cast<bool>(
            properties.queueFlags & vk::QueueFlagBits::eGraphics);
    }
    [[nodiscard]] bool has_compute_support() const noexcept {
        return static_cast<bool>(
            properties.queueFlags & vk::QueueFlagBits::eCompute);
    }
    [[nodiscard]] bool has_transfer_support() const noexcept {
        return static_cast<bool>(
            properties.queueFlags & vk::QueueFlagBits::eTransfer);
    }
    [[nodiscard]] bool has_sparse_binding_support() const noexcept {
        return static_cast<bool>(
            properties.queueFlags & vk::QueueFlagBits::eSparseBinding);
    }

  private:
    vk::QueueFamilyProperties properties;
    uint32_t index;
    bool present_support;
};
class Queue {
  public:
    Queue(vk::Queue queue, const Device &device);
};
} // namespace kovra
