#pragma once

#include "SDL_video.h"
#include <vulkan/vulkan.hpp>

namespace kovra {
class Instance {
  public:
    Instance(SDL_Window *window);

  private:
    vk::Instance instance;
    vk::DebugUtilsMessengerEXT debug_utils;
    // std::vector<PhysicalDevice> physical_devices;

    std::vector<const char *> req_instance_exts;
    std::vector<const char *> req_validation_layers;
};
} // namespace kovra
