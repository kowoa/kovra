#pragma once

#include <vulkan/vulkan.hpp>

namespace kovra {
// Forward declarations
class Device;

class Frame {
  public:
    Frame(const Device &device);

  private:
    // Signals when the swapchain is ready to present
    vk::UniqueSemaphore present_semaphore;
    // Signals when rendering is done
    // This happens when the command buffer gets submitted to the graphics queue
    vk::UniqueSemaphore render_semaphore;
    // Signals when render commands all finish execution
    vk::UniqueFence render_fence;
};
} // namespace kovra
