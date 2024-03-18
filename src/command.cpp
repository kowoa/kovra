#include "command.hpp"
#include "device.hpp"

namespace kovra {
CommandEncoder::CommandEncoder(const Device &device)
    : cmd_pool{device.get().allocateCommandBuffersUnique(
          vk::CommandBufferAllocateInfo{}
              .setCommandPool(device.get_command_pool())
              .setLevel(vk::CommandBufferLevel::ePrimary)
              .setCommandBufferCount(CMD_POOL_SIZE))},
      cmd_index{0} {}

vk::CommandBuffer CommandEncoder::finish() {
    auto result = cmd_pool[cmd_index].get();
    cmd_index = (cmd_index + 1) % CMD_POOL_SIZE;
    return result;
}

} // namespace kovra
