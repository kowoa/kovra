#pragma once

#include "command.hpp"
#include "descriptor.hpp"
#include "draw_context.hpp"
#include <memory>

namespace kovra {
// Forward declarations
class Device;
class GpuBuffer;

class Frame {
  public:
    explicit Frame(const Device &device);

    [[nodiscard]] vk::Fence get_render_fence() const noexcept {
        return render_fence.get();
    }

    void draw(const DrawContext &ctx);

  private:
    // Signals when the swapchain is ready to present
    vk::UniqueSemaphore present_semaphore;
    // Signals when rendering is done
    // This happens when the command buffer gets submitted to the graphics queue
    vk::UniqueSemaphore render_semaphore;
    // Signals when render commands all finish execution
    vk::UniqueFence render_fence;

    std::unique_ptr<CommandEncoder> cmd_encoder;
    std::unique_ptr<DescriptorAllocator> desc_allocator;
    std::unique_ptr<GpuBuffer> scene_buffer;

    void draw_background(const DrawContext &ctx);
};
} // namespace kovra
