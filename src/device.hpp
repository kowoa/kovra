#pragma once

#include "vk_mem_alloc.h"

#include "physical_device.hpp"

#include "buffer.hpp"
#include "command.hpp"
#include "queue.hpp"
#include "transfer_context.hpp"

namespace kovra {
class Device {
  public:
    Device(
        const Instance &instance,
        std::shared_ptr<PhysicalDevice> physical_device);
    ~Device();

    // Getters -----------------------------------------------------------------
    [[nodiscard]] const vk::Device &get() const noexcept {
        return device.get();
    }
    [[nodiscard]] const PhysicalDevice &get_physical_device() const noexcept {
        return *physical_device;
    }
    [[nodiscard]] const vk::Queue &get_graphics_queue() const noexcept {
        return graphics_queue->get();
    }
    [[nodiscard]] const vk::Queue &get_present_queue() const noexcept {
        return present_queue->get();
    }
    [[nodiscard]] const vk::Queue &get_transfer_queue() const noexcept {
        return transfer_queue->get();
    }
    [[nodiscard]] const vk::Queue &get_compute_queue() const noexcept {
        return compute_queue->get();
    }
    [[nodiscard]] uint32_t get_graphics_family_index() const noexcept {
        return graphics_queue->get_family_index();
    }
    [[nodiscard]] uint32_t get_present_family_index() const noexcept {
        return present_queue->get_family_index();
    }
    [[nodiscard]] uint32_t get_transfer_family_index() const noexcept {
        return transfer_queue->get_family_index();
    }
    [[nodiscard]] uint32_t get_compute_family_index() const noexcept {
        return compute_queue->get_family_index();
    }

    [[nodiscard]] const VmaAllocator &get_allocator() const noexcept {
        return *allocator.get();
    }
    [[nodiscard]] const vk::CommandPool &get_command_pool() const noexcept {
        return command_pool.get();
    }
    //--------------------------------------------------------------------------

    [[nodiscard]] CommandEncoder create_command_encoder() const;
    [[nodiscard]] std::unique_ptr<GpuBuffer> create_buffer(
        vk::DeviceSize size, vk::BufferUsageFlags buffer_usage,
        VmaMemoryUsage alloc_usage, VmaAllocationCreateFlags alloc_flags) const;
    /*
      [[nodiscard]] GpuImage create_image(
          const vk::ImageCreateInfo &image_info,
          VmaMemoryUsage memory_usage) const;
    */

  private:
    vk::UniqueDevice device;
    std::shared_ptr<PhysicalDevice> physical_device;

    std::unique_ptr<Queue> graphics_queue;
    std::unique_ptr<Queue> present_queue;
    std::unique_ptr<Queue> transfer_queue;
    std::unique_ptr<Queue> compute_queue;

    std::shared_ptr<VmaAllocator> allocator;
    vk::UniqueCommandPool command_pool;
    std::unique_ptr<TransferContext> transfer_context;
};
} // namespace kovra
