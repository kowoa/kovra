#pragma once

#include "compute_pass.hpp"
#include "render_pass.hpp"

namespace kovra {
// Forward declarations
class Device;

class CommandEncoder {
  public:
    CommandEncoder(const Device &device);
    ~CommandEncoder();
    CommandEncoder(const CommandEncoder &) = delete;
    CommandEncoder &operator=(const CommandEncoder &) = delete;

    [[nodiscard]] RenderPass
    begin_render_pass(const RenderPassDescriptor &desc);
    [[nodiscard]] ComputePass begin_compute_pass();
    [[nodiscard]] vk::CommandBuffer finish();
    [[nodiscard]] vk::CommandBuffer get_cmd_buffer() const noexcept {
        return cmd_buffers.at(cmd_index).get();
    }

  private:
    static constexpr const uint32_t CMD_BUFFER_COUNT = 1;

    std::vector<vk::UniqueCommandBuffer> cmd_buffers;
    uint32_t cmd_index;
    bool is_recording;

    std::optional<vk::CommandBuffer> begin_recording();
    std::optional<vk::CommandBuffer> end_recording();
};
} // namespace kovra
