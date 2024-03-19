#pragma once

#include "compute_pass.hpp"
#include "render_pass.hpp"

namespace kovra {
// Forward declarations
class Device;

class CommandEncoder {
  public:
    CommandEncoder(const Device &device);

    [[nodiscard]] RenderPass
    begin_render_pass(const RenderPassDescriptor &desc);
    [[nodiscard]] ComputePass begin_compute_pass();
    [[nodiscard]] vk::CommandBuffer finish();

  private:
    static constexpr const uint32_t CMD_POOL_SIZE = 1;

    std::vector<vk::UniqueCommandBuffer> cmd_buffers;
    uint32_t cmd_index;
    bool is_recording;

    std::optional<vk::CommandBuffer> begin_recording();
    std::optional<vk::CommandBuffer> end_recording();
};
} // namespace kovra
