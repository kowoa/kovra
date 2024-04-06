#pragma once

#include "camera.hpp"
#include "gpu_data.hpp"
#include "profiling.hpp"

namespace kovra {
// Forward declarations
class Device;
class Swapchain;
class GpuImage;
class RenderResources;
struct RenderObject;
struct MaterialInstance;
class Cubemap;

// WARNING: Do not store this struct in any class as a member.
// It contains references to objects that may be destroyed.
struct DrawContext
{
    const Device &device;
    const RenderResources &render_resources;
    const Camera &camera;

    Swapchain &swapchain;
    GpuImage &draw_image;
    Cubemap &skybox;

    // This vector will be filled each frame with opaque render objects
    std::vector<RenderObject> opaque_objects;
    // This vector will be filled each frame with transparent render objects
    std::vector<RenderObject> transparent_objects;

    const uint32_t frame_number;
    const float render_scale = 1.0f;

    const GpuSceneData scene_data;

    // Profiling
    RendererStats &stats;
};
} // namespace kovra
