#pragma once

#include "vk_mem_alloc.h"

#include "upload_context.hpp"
#include <memory>

namespace kovra {
class Context {
  public:
    Context(SDL_Window *window);
    ~Context();

    [[nodiscard]] const vk::Instance &get_instance() const noexcept {
        return instance.get()->get();
    }
    [[nodiscard]] const vk::SurfaceKHR &get_surface() const noexcept {
        return surface.get()->get();
    }
    [[nodiscard]] const vk::PhysicalDevice &
    get_physical_device() const noexcept {
        return physical_device.get()->get();
    }
    [[nodiscard]] const vk::Device &get_device() const noexcept {
        return device.get()->get();
    }
    [[nodiscard]] const vk::Queue &get_graphics_queue() const noexcept {
        return graphics_queue.get();
    }
    [[nodiscard]] const vk::Queue &get_present_queue() const noexcept {
        return present_queue.get();
    }
    [[nodiscard]] const VmaAllocator &get_allocator() const noexcept {
        return *allocator.get();
    }
    [[nodiscard]] const vk::CommandPool &get_command_pool() const noexcept {
        return command_pool.get();
    }

  private:
    std::unique_ptr<Instance> instance;
    std::unique_ptr<Surface> surface;
    std::shared_ptr<PhysicalDevice>
        physical_device; // Instance also has ownership of all physical devices
    std::shared_ptr<Device> device;
    Queue graphics_queue;
    Queue present_queue;
    QueueFamily graphics_queue_family;
    QueueFamily present_queue_family;
    std::unique_ptr<VmaAllocator> allocator;
    vk::UniqueCommandPool command_pool;
    UploadContext upload_context;
};
} // namespace kovra
