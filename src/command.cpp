#include "command.hpp"
#include "device.hpp"
#include "spdlog/spdlog.h"
#include "utils.hpp"

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
CommandEncoder::begin_render_pass(const RenderPassDescriptor &desc)
{
    begin_recording();
    return RenderPass{ desc, cmd_buffers.at(cmd_index).get() };
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
        return {};
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
  vk::ImageLayout new_layout
) const
{
    utils::transition_image_layout(
      get_current_cmd(), image, aspect, old_layout, new_layout
    );
}
void
CommandEncoder::copy_image_to_image(
  vk::Image src,
  vk::Image dst,
  vk::Extent2D src_size,
  vk::Extent2D dst_size
) const
{
    utils::copy_image_to_image(get_current_cmd(), src, dst, src_size, dst_size);
}

} // namespace kovra
