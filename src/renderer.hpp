#pragma once

#include "context.hpp"
#include "frame.hpp"

namespace kovra {
class Renderer {
  public:
    explicit Renderer(SDL_Window *window);
    ~Renderer();

    void draw_frame(const Camera &camera);

  private:
    static constexpr const uint32_t FRAME_OVERLAP = 2;

    Context context;

    uint32_t frame_number;
    std::vector<std::unique_ptr<Frame>> frames;
    std::unordered_map<std::string, vk::UniqueDescriptorSetLayout>
        desc_set_layouts;

    [[nodiscard]] Frame &get_current_frame() const noexcept {
        return *frames.at(frame_number % frames.size());
    }
};
} // namespace kovra
