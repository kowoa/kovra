#include "command.hpp"
#include "device.hpp"

namespace kovra {
CommandEncoder::CommandEncoder(const Device &device)
    : cmd_buffers{device.get().allocateCommandBuffersUnique(
          vk::CommandBufferAllocateInfo{}
              .setCommandPool(device.get_command_pool())
              .setLevel(vk::CommandBufferLevel::ePrimary)
              .setCommandBufferCount(CMD_POOL_SIZE))},
      cmd_index{0}, is_recording{false} {}

ComputePass CommandEncoder::begin_compute_pass() {
    begin_recording();
    return ComputePass{cmd_buffers.at(cmd_index).get()};
}

vk::CommandBuffer CommandEncoder::finish() {
    auto cmd = end_recording();
    if (!cmd.has_value()) {
        throw std::runtime_error(
            "Tried to finish command encoder but never began it");
    }
    cmd_index = (cmd_index + 1) % CMD_POOL_SIZE;
    return cmd.value();
}

std::optional<vk::CommandBuffer> CommandEncoder::begin_recording() {
    if (is_recording) {
        return {};
    }

    auto cmd = cmd_buffers.at(cmd_index).get();
    // Reset the command buffer
    cmd.reset({});
    // Begin recording the command buffer
    cmd.begin(vk::CommandBufferBeginInfo{}.setFlags(
        vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    is_recording = true;
    return cmd;
}

std::optional<vk::CommandBuffer> CommandEncoder::end_recording() {
    if (!is_recording) {
        return {};
    }

    auto cmd = cmd_buffers.at(cmd_index).get();
    cmd.end();
    is_recording = false;
    return cmd;
}

} // namespace kovra
