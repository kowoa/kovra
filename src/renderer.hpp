#pragma once

#include "vk_mem_alloc.h"

#include "context.hpp"

namespace kovra {
// Forward declarations
class Camera;
class Frame;

class Renderer {
  public:
    Renderer() = delete;
    Renderer(const Renderer &) = delete;

    explicit Renderer(SDL_Window *window);
    ~Renderer();

    void draw_frame(const Camera &camera);

  private:
    static constexpr const uint32_t FRAME_OVERLAP = 2;

    std::unique_ptr<Context> context;

    uint32_t frame_number;
    std::vector<std::unique_ptr<Frame>> frames;

    // Resources
    std::unordered_map<std::string, vk::DescriptorSetLayout> desc_set_layouts;
    std::unordered_map<vk::Filter, vk::Sampler> samplers;
    std::shared_ptr<GpuImage> background_image;

    [[nodiscard]] Frame &get_current_frame() const noexcept {
        return *frames.at(frame_number % frames.size());
    }
};
} // namespace kovra
