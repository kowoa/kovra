#pragma once

#define VULKAN_HPP_EXCEPTIONS

#include "context.hpp"
#include "frame.hpp"
#include "image.hpp"

namespace kovra {
// Forward declarations
// class Camera;

class Renderer {
  public:
    explicit Renderer(SDL_Window *window);
    ~Renderer();
    Renderer() = delete;
    Renderer(const Renderer &) = delete;

    void draw_frame(const Camera &camera);

  private:
    static constexpr const uint32_t FRAME_OVERLAP = 2;

    std::unique_ptr<Context> context;

    uint32_t frame_number;
    std::vector<std::unique_ptr<Frame>> frames;

    // Resources
    std::unordered_map<std::string, vk::DescriptorSetLayout> desc_set_layouts;
    std::unordered_map<vk::Filter, vk::UniqueSampler> samplers;
    std::shared_ptr<GpuImage> background_image;

    [[nodiscard]] Frame &get_current_frame() const noexcept {
        return *frames.at(frame_number % frames.size());
    }
};
} // namespace kovra
