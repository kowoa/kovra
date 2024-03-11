#pragma once

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#include "SDL_video.h"
#include <vulkan/vulkan.hpp>

namespace kovra {
class Instance {
  public:
    Instance(SDL_Window *window);
    // std::vector<PhysicalDevice> enumerate_physical_devices();

    vk::UniqueInstance instance;

  private:
    vk::UniqueDebugUtilsMessengerEXT debug_utils;
    //    std::vector<PhysicalDevice> physical_devices;
};
} // namespace kovra
