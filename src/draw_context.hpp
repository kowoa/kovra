#pragma once

#include "camera.hpp"

namespace kovra {
// Forward declarations
class Device;
class Swapchain;
class GpuImage;
class RenderResources;

struct DrawContext
{
    const Device &device;
    Swapchain &swapchain;
    const uint32_t frame_number;
    const Camera &camera;
    const RenderResources &render_resources;
    GpuImage &draw_image;
    const float render_scale = 1.0f;
};
} // namespace kovra
