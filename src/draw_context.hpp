#pragma once

#include "camera.hpp"
#include "gpu_data.hpp"

namespace kovra {
// Forward declarations
class Device;
class Swapchain;
class GpuImage;
class RenderResources;
struct RenderObject;
struct MaterialInstance;

// WARNING: Do not store this struct in any class as a member
struct DrawContext
{
    const Device &device;
    const RenderResources &render_resources;
    const Camera &camera;

    Swapchain &swapchain;
    GpuImage &draw_image;

    // This vector will be filled each frame with opaque render objects
    std::vector<RenderObject> opaque_objects;

    const uint32_t frame_number;
    const float render_scale = 1.0f;

    const GpuSceneData scene_data;
};
} // namespace kovra
