#pragma once

#include "camera.hpp"
#include "instance.hpp"

namespace kovra {
// Forward declarations
class Device;
class Swapchain;
class GpuImage;
class RenderResources;

struct DrawContext
{
    std::shared_ptr<Device> device;
    Swapchain &swapchain;
    uint32_t frame_number;
    Camera &camera;
    std::shared_ptr<RenderResources> render_resources;
    GpuImage &draw_image;
    float render_scale = 1.0f;
};
} // namespace kovra
