#pragma once

#include "render_pass.hpp"
#include <memory>

namespace kovra {
// Forward declarations
class Device;

class CommandEncoder {
  public:
    CommandEncoder(std::shared_ptr<Device> device);

    RenderPass begin_render_pass(const RenderPassDescriptor &desc);
    vk::UniqueCommandBuffer finish();

  private:
    std::shared_ptr<Device> device;
    vk::UniqueCommandBuffer cmd;
};
} // namespace kovra
