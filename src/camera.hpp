#pragma once

#include "glm/glm.hpp"
#include "gpu_data.hpp"

namespace kovra {
class Camera {
  public:
    Camera();

    void set_position(const glm::vec3 &pos) noexcept;
    void look_at(const glm::vec3 &target) noexcept;
    void zoom(glm::f32 delta) noexcept;
    void rotate(
        glm::vec2 last_mouse_pos, glm::vec2 curr_mouse_pos,
        glm::f32 viewport_width, glm::f32 viewport_height) noexcept;

    [[nodiscard]] glm::mat4x4 get_viewproj_mat(
        glm::f32 viewport_width, glm::f32 viewport_height) const noexcept;
    [[nodiscard]] glm::mat4x4 get_view_mat() const noexcept;
    [[nodiscard]] glm::mat4x4 get_proj_mat(
        glm::f32 viewport_width, glm::f32 viewport_height) const noexcept;
    [[nodiscard]] GpuCameraData as_gpu_data() const noexcept;

  private:
    glm::vec3 position;
    glm::vec3 forward;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 world_up;
    glm::f32 fov_y_deg;
    glm::f32 near;
    glm::f32 far;
    glm::vec3 pivot;
};
} // namespace kovra
