#pragma once

#include <vulkan/vulkan.hpp>

namespace kovra {
class ComputePass {
  public:
    ComputePass(const vk::CommandBuffer &cmd) : cmd{cmd} {}

  private:
    const vk::CommandBuffer &cmd;
};
} // namespace kovra
