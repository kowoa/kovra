#pragma once

#include "SDL_video.h"
#include <vulkan/vulkan.hpp>

namespace kovra {
class Instance {
  public:
    Instance(SDL_Window *window);

  private:
    vk::UniqueInstance instance;
    // vk::UniqueDebugUtilsMessengerEXT debug_utils;
    //  std::vector<PhysicalDevice> physical_devices;
};
} // namespace kovra
