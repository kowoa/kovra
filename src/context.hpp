#pragma once

// #define VMA_IMPLEMENTATION
// #include "vk_mem_alloc.h"
#include "instance.hpp"

namespace kovra {
class Context {
  public:
    Context(SDL_Window *window);

    Instance instance;
    // Surface surface;
    // PhysicalDevice physical_device;
    // Device device;
    //  Queue graphics_queue;
    //  Queue present_queue;
    //  QueueFamily graphics_queue_family;
    //  QueueFamily present_queue_family;
    //  VmaAllocator allocator;
};
} // namespace kovra
