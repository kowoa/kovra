#pragma once

#include "render_pass.hpp"

namespace kovra {
// Forward declarations
class Device;

class CommandEncoder {
  public:
    CommandEncoder(const Device &device);

    RenderPass begin_render_pass(const RenderPassDescriptor &desc);
    vk::UniqueCommandBuffer finish();

  private:
    vk::UniqueCommandBuffer cmd;
};
} // namespace kovra
