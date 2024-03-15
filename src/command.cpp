#include "command.hpp"
#include "device.hpp"

namespace kovra {
CommandEncoder::CommandEncoder(const Device &device)
    : cmd{std::move(device.get().allocateCommandBuffersUnique(
          vk::CommandBufferAllocateInfo{}
              .setCommandPool(device.get_command_pool())
              .setLevel(vk::CommandBufferLevel::ePrimary)
              .setCommandBufferCount(1))[0])} {}

vk::UniqueCommandBuffer CommandEncoder::finish() { return std::move(cmd); }
} // namespace kovra
