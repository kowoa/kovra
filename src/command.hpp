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
    CommandEncoder(const CommandEncoder &) = delete;
    CommandEncoder &operator=(const CommandEncoder &) = delete;

    [[nodiscard]] RenderPass begin_render_pass(const RenderPassDescriptor &desc
    );
    [[nodiscard]] ComputePass begin_compute_pass();
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
