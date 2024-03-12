#pragma once

// #define VMA_IMPLEMENTATION
// #include "vk_mem_alloc.h"
#include "surface.hpp"
#include <memory>

namespace kovra {
class Context {
  public:
    Context(SDL_Window *window);

    std::unique_ptr<Instance> instance;
    std::unique_ptr<Surface> surface;
    std::weak_ptr<PhysicalDevice>
        physical_device; // Instance has ownership of all physical devices
    std::shared_ptr<Device> device;
    //   Queue graphics_queue;
    //   Queue present_queue;
    //   QueueFamily graphics_queue_family;
    //   QueueFamily present_queue_family;
    //   VmaAllocator allocator;
};
} // namespace kovra
