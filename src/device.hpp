#pragma once

#include "vk_mem_alloc.h"

#include "command.hpp"
#include "physical_device.hpp"
#include "queue.hpp"
#include "upload_context.hpp"

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
    [[nodiscard]] const vk::Queue &get_graphics_queue() const noexcept {
        return graphics_queue->get();
    }
    [[nodiscard]] const vk::Queue &get_present_queue() const noexcept {
        return present_queue->get();
    }
    [[nodiscard]] uint32_t get_graphics_family_index() const noexcept {
        return graphics_queue->get_family_index();
    }
    [[nodiscard]] uint32_t get_present_family_index() const noexcept {
        return present_queue->get_family_index();
    }

    [[nodiscard]] const VmaAllocator &get_allocator() const noexcept {
        return *allocator.get();
    }
    [[nodiscard]] const vk::CommandPool &get_command_pool() const noexcept {
        return command_pool.get();
    }
    //--------------------------------------------------------------------------

    [[nodiscard]] CommandEncoder create_command_encoder() const;
    [[nodiscard]] vk::UniqueCommandBuffer create_command_buffer() const;

  private:
    vk::UniqueDevice device;
    std::shared_ptr<PhysicalDevice> physical_device;
    std::unique_ptr<Queue> graphics_queue;
    std::unique_ptr<Queue> present_queue;
    std::unique_ptr<VmaAllocator> allocator;
    vk::UniqueCommandPool command_pool;
    std::unique_ptr<UploadContext> upload_context;
};
} // namespace kovra
