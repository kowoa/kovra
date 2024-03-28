#pragma once

#define VULKAN_HPP_EXCEPTIONS

#include "context.hpp"
#include "frame.hpp"
#include "image.hpp"

namespace kovra {
// Forward declarations
class RenderResources;

class Renderer {
  public:
    explicit Renderer(SDL_Window *window);
    ~Renderer();
    Renderer() = delete;
    Renderer(const Renderer &) = delete;

    void draw_frame(Camera &camera);
    [[nodiscard]] const Context &get_context() const noexcept {
        return *context;
    }
    [[nodiscard]] uint32_t get_frame_number() const noexcept {
        return frame_number;
    }

  private:
    static constexpr const uint32_t FRAME_OVERLAP = 2;

    std::unique_ptr<Context> context;

    uint32_t frame_number;
    std::vector<std::unique_ptr<Frame>> frames;

    // Resources
    std::shared_ptr<RenderResources> render_resources;
    std::shared_ptr<GpuImage> background_image;

    // ImGui
    VkDescriptorPool imgui_pool;

    [[nodiscard]] Frame &get_current_frame() const noexcept {
        return *frames.at(frame_number % frames.size());
    }

    void init_imgui(SDL_Window *window, const Context &ctx);
};
} // namespace kovra
