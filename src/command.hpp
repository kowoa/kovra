#pragma once

#include "compute_pass.hpp"
#include "render_pass.hpp"

namespace kovra {
// Forward declarations
class Device;

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
    [[nodiscard]] RenderPass begin_render_pass(const RenderPassDescriptor &desc
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
      vk::ImageLayout new_layout
    ) const;
    void copy_image_to_image(
      vk::Image src,
      vk::Image dst,
      vk::Extent2D src_size,
      vk::Extent2D dst_size
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
