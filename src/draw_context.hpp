#pragma once

#include "camera.hpp"
#include "instance.hpp"

namespace kovra {
// Forward declarations
class Device;
class Swapchain;
class GpuImage;

struct DrawContext {
    std::shared_ptr<Device> device;
    std::shared_ptr<Swapchain> swapchain;
    uint32_t frame_number;
    const Camera &camera;
    const std::unordered_map<std::string, vk::DescriptorSetLayout>
        &desc_set_layouts;
    std::shared_ptr<GpuImage> background_image;
};
} // namespace kovra
