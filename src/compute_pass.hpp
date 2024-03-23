#pragma once

#include <vulkan/vulkan.hpp>

namespace kovra {
class ComputePass {
  public:
    ComputePass(const vk::CommandBuffer &cmd) : cmd{cmd} {}

    [[nodiscard]] const vk::CommandBuffer &get_cmd() const { return cmd; }

  private:
    const vk::CommandBuffer &cmd;
};
} // namespace kovra
