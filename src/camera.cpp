#include "camera.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace kovra {
Camera::Camera()
  : position{ glm::vec3{ 0.0f, 0.0f, 5.0f } }
  , forward{ glm::vec3{ 0.0f, 0.0f, -1.0f } }
  , up{ glm::vec3{ 0.0f, 1.0f, 0.0f } }
  , right{ glm::vec3{ 1.0f, 0.0f, 0.0f } }
  , world_up{ glm::vec3{ 0.0f, 1.0f, 0.0f } }
  , fov_y_deg{ 45.0f }
  , near{ 0.1f }
  , far{ 200.0f }
  , pivot{ glm::vec3{ 0.0f, 0.0f, 0.0f } }
{
}

void
Camera::set_position(const glm::vec3 &pos) noexcept
{
    position = pos;
    look_at(pivot);
}

void
Camera::look_at(const glm::vec3 &target) noexcept
{
    if (target == position) {
        return;
    }
    pivot = target;
    forward = glm::normalize(target - position);
    right = glm::normalize(glm::cross(forward, world_up));
    up = glm::normalize(glm::cross(right, forward));
}

void
Camera::mouse_zoom(glm::f32 mouse_wheel_delta_y) noexcept
{
    auto new_pos = position + forward * mouse_wheel_delta_y;
    if (glm::distance(new_pos, pivot) > 0.1f) {
        set_position(new_pos);
    }
}

void
Camera::mouse_rotate(
  glm::vec2 prev_mouse_pos,
  glm::vec2 curr_mouse_pos,
  glm::f32 viewport_width,
  glm::f32 viewport_height
) noexcept
{
    // Get the homogeneous positions of the camera eye and pivot
    auto pos = glm::vec4(position, 1.0f);
    auto piv = glm::vec4(pivot, 1.0f);

    // Calculate the amount of rotation given the mouse movement
    // Left to right = 2*PI = 360 deg
    auto delta_angle_x = 2.0f * glm::pi<glm::f32>() / viewport_width;
    // Top to bottom = PI = 180 deg
    auto delta_angle_y = glm::pi<glm::f32>() / viewport_height;
    auto angle_x = (prev_mouse_pos.x - curr_mouse_pos.x) * delta_angle_x;
    auto angle_y = (prev_mouse_pos.y - curr_mouse_pos.y) * delta_angle_y;

    // Handle case where the camera's forward is the same as its up
    auto cos_angle = glm::dot(forward, up);
    if (cos_angle * glm::sign(delta_angle_y) > 0.99f) {
        delta_angle_y = 0.0f;
    }

    // Rotate the camera around the pivot point on the up axis
    auto rot_x = glm::rotate(glm::mat4(1.0f), angle_x, up);
    pos = (rot_x * (pos - piv)) + piv;

    // Rotate the camera around the pivot point on the right axis
    auto rot_y = glm::rotate(glm::mat4(1.0f), angle_y, right);
    pos = (rot_y * (pos - piv)) + piv;

    // Update camera position
    set_position(glm::vec3(pos));
}

[[nodiscard]] glm::mat4x4
Camera::get_viewproj_mat(glm::f32 viewport_width, glm::f32 viewport_height)
  const noexcept
{
    return get_proj_mat(viewport_width, viewport_height) * get_view_mat();
}

[[nodiscard]] glm::mat4x4
Camera::get_view_mat() const noexcept
{
    return glm::lookAtRH(position, pivot, up);
}

[[nodiscard]] glm::mat4x4
Camera::get_proj_mat(glm::f32 viewport_width, glm::f32 viewport_height)
  const noexcept
{
    auto proj = glm::perspectiveRH(
      glm::radians(fov_y_deg), viewport_width / viewport_height, near, far
    );
    proj[1][1] *= -1.0f; // Flip the Y axis
    return proj;
}

} // namespace kovra
