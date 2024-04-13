#include "command.hpp"
#include "device.hpp"
#include "image.hpp"
#include "utils.hpp"

#include "spdlog/spdlog.h"

namespace kovra {
CommandEncoder::CommandEncoder(const Device &device)
  : cmd_buffers{ device.get().allocateCommandBuffersUnique(
      vk::CommandBufferAllocateInfo{}
        .setCommandPool(device.get_command_pool())
        .setLevel(vk::CommandBufferLevel::ePrimary)
        .setCommandBufferCount(CMD_BUFFER_COUNT)
    ) }
  , cmd_index{ 0 }
  , is_recording{ false }
{
    spdlog::debug("CommandEncoder::CommandEncoder()");
}

CommandEncoder::~CommandEncoder()
{
    spdlog::debug("CommandEncoder::~CommandEncoder()");
    for (auto &cmd_buffer : cmd_buffers) {
        cmd_buffer.reset();
    }
    cmd_buffers.clear();
}

ComputePass
CommandEncoder::begin_compute_pass()
{
    begin_recording();
    return ComputePass{ cmd_buffers.at(cmd_index).get() };
}

RenderPass
CommandEncoder::begin_render_pass(const RenderPassCreateInfo &info)
{
    begin_recording();
    return RenderPass{ info, cmd_buffers.at(cmd_index).get() };
}

void
CommandEncoder::begin()
{
    begin_recording();
}

vk::CommandBuffer
CommandEncoder::finish()
{
    auto cmd = end_recording();
    if (!cmd.has_value()) {
        throw std::runtime_error(
          "Tried to finish command encoder but never began it"
        );
    }
    cmd_index = (cmd_index + 1) % CMD_BUFFER_COUNT;
    return cmd.value();
}

std::optional<vk::CommandBuffer>
CommandEncoder::begin_recording()
{
    if (is_recording) {
        return std::nullopt;
    }

    auto cmd = get_current_cmd();
    // Reset the command buffer
    cmd.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
    // Begin recording the command buffer
    cmd.begin(vk::CommandBufferBeginInfo{}.setFlags(
      vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    ));
    is_recording = true;
    return cmd;
}

std::optional<vk::CommandBuffer>
CommandEncoder::end_recording()
{
    if (!is_recording) {
        return {};
    }

    auto cmd = get_current_cmd();
    cmd.end();
    is_recording = false;
    return cmd;
}

void
CommandEncoder::transition_image_layout(
  const vk::Image &image,
  vk::ImageAspectFlagBits aspect,
  vk::ImageLayout old_layout,
  vk::ImageLayout new_layout,
  int layer_count,
  int level_count
) const
{
    utils::transition_image_layout(
      get_current_cmd(),
      image,
      aspect,
      old_layout,
      new_layout,
      layer_count,
      level_count
    );
}

void
CommandEncoder::transition_image_layout(
  GpuImage &image,
  vk::ImageLayout old_layout,
  vk::ImageLayout new_layout
) const
{
    image.transition_layout(get_current_cmd(), old_layout, new_layout);
}

void
CommandEncoder::resolve_image(
  const vk::Image &src,
  const vk::ImageLayout &src_layout,
  const vk::Image &dst,
  const vk::ImageLayout &dst_layout,
  const vk::ImageResolve &region
) const
{
    get_current_cmd().resolveImage(src, src_layout, dst, dst_layout, region);
}

void
CommandEncoder::copy_image_to_image(
  vk::Image src,
  vk::Image dst,
  vk::Extent2D src_size,
  vk::Extent2D dst_size,
  vk::ImageAspectFlagBits src_aspect,
  vk::ImageAspectFlagBits dst_aspect,
  vk::Filter filter
) const
{
    utils::copy_image_to_image(
      get_current_cmd(),
      src,
      dst,
      src_size,
      dst_size,
      src_aspect,
      dst_aspect,
      filter
    );
}
void
CommandEncoder::clear_color_image(
  const vk::Image &image,
  const vk::ImageLayout &layout,
  const vk::ClearColorValue &color
) const
{
    get_current_cmd().clearColorImage(
      image,
      layout,
      color,
      vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
    );
}

void
CommandEncoder::clear_depth_image(
  const vk::Image &image,
  const vk::ImageLayout &layout
) const
{
    get_current_cmd().clearDepthStencilImage(
      image,
      layout,
      vk::ClearDepthStencilValue{ 1.0f, 0 },
      vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 }
    );
}

} // namespace kovra
