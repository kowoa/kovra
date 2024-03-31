#pragma once

#include "draw_context.hpp"
#include <memory>

namespace kovra {
// Forward declarations
class Device;
class GpuBuffer;
class CommandEncoder;
class DescriptorAllocator;
class ComputePass;
class RenderPass;

class Frame
{
  public:
    explicit Frame(const Device &device);
    ~Frame();
    Frame() = delete;
    Frame(const Frame &) = delete;

    [[nodiscard]] vk::Fence get_render_fence() const noexcept
    {
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

    vk::Extent2D draw_extent;

    void draw_background(ComputePass &pass, const DrawContext &ctx);
    void draw_meshes(
      RenderPass &pass,
      const DrawContext &ctx,
      const vk::DescriptorSet &scene_desc_set
    );
    void draw_grid(
      RenderPass &pass,
      const DrawContext &ctx,
      const vk::DescriptorSet &scene_desc_set
    );
    void present(uint32_t swapchain_image_index, const DrawContext &ctx);
};
} // namespace kovra
