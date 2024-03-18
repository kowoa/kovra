#pragma once

#include "render_pass.hpp"

namespace kovra {
// Forward declarations
class Device;

class CommandEncoder {
  public:
    CommandEncoder(const Device &device);

    RenderPass begin_render_pass(const RenderPassDescriptor &desc);
    vk::CommandBuffer finish();

  private:
    static constexpr const uint32_t CMD_POOL_SIZE = 1;

    std::vector<vk::UniqueCommandBuffer> cmd_pool;
    uint32_t cmd_index;
};
} // namespace kovra
