#pragma once

#include "SDL_video.h"
#include "camera.hpp"
#include "renderer.hpp"
#include <memory>

namespace kovra {
class App {
  public:
    App();
    ~App();
    void run();

  private:
    SDL_Window *window;
    std::unique_ptr<Renderer> renderer;

    // Camera
    Camera camera;
    glm::vec2 prev_mouse_pos{0.0f, 0.0f};
    bool camera_movable{false};
};
} // namespace kovra
