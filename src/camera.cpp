#include "camera.hpp"

namespace kovra {
Camera::Camera()
    : position{glm::vec3{0.0f, 0.0f, 5.0f}},
      forward{glm::vec3{0.0f, 0.0f, -1.0f}}, up{glm::vec3{0.0f, 1.0f, 0.0f}},
      right{glm::vec3{1.0f, 0.0f, 0.0f}}, world_up{glm::vec3{0.0f, 1.0f, 0.0f}},
      fov_y_deg{45.0f}, near{0.1f}, far{100.0f},
      pivot{glm::vec3{0.0f, 0.0f, 0.0f}} {}
} // namespace kovra
