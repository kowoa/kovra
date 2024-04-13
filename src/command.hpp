#pragma once

#include "compute_pass.hpp"
#include "render_pass.hpp"

namespace kovra {
// Forward declarations
class Device;
class GpuImage;

class CommandEncoder
{
  public:
    CommandEncoder(const Device &device);
    ~CommandEncoder();
    CommandEncoder() = delete;
    CommandEncoder(const CommandEncoder &) = delete;
    CommandEncoder &operator=(const CommandEncoder &) = delete;
    CommandEncoder(CommandEncoder &&) noexcept = delete;
    CommandEncoder &operator=(CommandEncoder &&) noexcept = delete;

    // Begin render pass and begin recording commands if not already recording
    [[nodiscard]] RenderPass begin_render_pass(const RenderPassCreateInfo &info
    );
    // Begin compute pass and begin recording commands if not already recording
    [[nodiscard]] ComputePass begin_compute_pass();
    // Begin recording commands
    void begin();
    // End recording commands
    [[nodiscard]] vk::CommandBuffer finish();

    void transition_image_layout(
      const vk::Image &image,
      vk::ImageAspectFlagBits aspect,
      vk::ImageLayout old_layout,
      vk::ImageLayout new_layout,
      int layer_count = 1,
      int level_count = 1
    ) const;
    void transition_image_layout(
      GpuImage &image,
      vk::ImageLayout old_layout,
      vk::ImageLayout new_layout
    ) const;
    void resolve_image(
      const vk::Image &src,
      const vk::ImageLayout &src_layout,
      const vk::Image &dst,
      const vk::ImageLayout &dst_layout,
      const vk::ImageResolve &region
    ) const;
    void copy_image_to_image(
      vk::Image src,
      vk::Image dst,
      vk::Extent2D src_size,
      vk::Extent2D dst_size,
      vk::ImageAspectFlagBits src_aspect = vk::ImageAspectFlagBits::eColor,
      vk::ImageAspectFlagBits dst_aspect = vk::ImageAspectFlagBits::eColor,
      vk::Filter filter = vk::Filter::eLinear
    ) const;
    void clear_color_image(
      const vk::Image &image,
      const vk::ImageLayout &layout,
      const vk::ClearColorValue &color =
        vk::ClearColorValue{ 0.0f, 0.0f, 0.0f, 0.0f }
    ) const;
    // Clear depth image to 1.0f
    // NOTE: layout can only be either eGeneral or eTransferDstOptimal
    void clear_depth_image(
      const vk::Image &image,
      const vk::ImageLayout &layout
    ) const;

  private:
    static constexpr const uint32_t CMD_BUFFER_COUNT = 1;

    std::vector<vk::UniqueCommandBuffer> cmd_buffers;
    uint32_t cmd_index;
    bool is_recording;

    std::optional<vk::CommandBuffer> begin_recording();
    std::optional<vk::CommandBuffer> end_recording();

    [[nodiscard]] vk::CommandBuffer get_current_cmd() const noexcept
    {
        return cmd_buffers.at(cmd_index).get();
    }
};
} // namespace kovra
