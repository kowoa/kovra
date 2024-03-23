#pragma once

#include "camera.hpp"
#include "instance.hpp"

namespace kovra {
// Forward declarations
class Device;
class Swapchain;
class GpuImage;
class RenderResources;

struct DrawContext {
    std::shared_ptr<Device> device;
    std::shared_ptr<Swapchain> swapchain;
    uint32_t frame_number;
    const Camera &camera;
    std::shared_ptr<RenderResources> render_resources;
    std::shared_ptr<GpuImage> background_image;
};
} // namespace kovra
