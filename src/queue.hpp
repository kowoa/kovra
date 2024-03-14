#pragma once

#include "device.hpp"

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

    bool operator==(const QueueFamily &other) const {
        return index == other.index;
    }

  private:
    vk::QueueFamilyProperties properties;
    uint32_t index;
    bool present_support;
};
class Queue {
  public:
    Queue(QueueFamily family, const std::shared_ptr<Device> &device);

    [[nodiscard]] const vk::Queue &get() const noexcept { return queue; }
    [[nodiscard]] uint32_t get_family_index() const noexcept {
        return family.get_index();
    }

  private:
    std::weak_ptr<Device> device;
    QueueFamily family;
    vk::Queue queue;
};
} // namespace kovra
