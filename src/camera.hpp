#pragma once

#include "glm/glm.hpp"

namespace kovra {
class Camera
{
  public:
    Camera();

    void set_position(const glm::vec3 &pos) noexcept;
    void look_at(const glm::vec3 &target) noexcept;

    // Mouse processing
    void mouse_zoom(glm::f32 mouse_wheel_delta_y) noexcept;
    void mouse_rotate(
      glm::vec2 prev_mouse_pos,
      glm::vec2 curr_mouse_pos,
      glm::f32 viewport_width,
      glm::f32 viewport_height
    ) noexcept;

    [[nodiscard]] glm::mat4x4 get_viewproj_mat(
      glm::f32 viewport_width,
      glm::f32 viewport_height
    ) const noexcept;
    [[nodiscard]] glm::mat4x4 get_view_mat() const noexcept;
    [[nodiscard]] glm::mat4x4 get_proj_mat(
      glm::f32 viewport_width,
      glm::f32 viewport_height
    ) const noexcept;
    [[nodiscard]] glm::vec3 get_position() const noexcept { return position; }
    [[nodiscard]] glm::f32 get_near() const noexcept { return near; }
    [[nodiscard]] glm::f32 get_far() const noexcept { return far; }

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
