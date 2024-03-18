#pragma once

#include <memory>
namespace kovra {
// Forward declarations
class Device;
class Swapchain;

struct DrawContext {
    std::shared_ptr<Device> device;
    std::shared_ptr<Swapchain> swaphchain;
    uint32_t frame_number;
};
} // namespace kovra
