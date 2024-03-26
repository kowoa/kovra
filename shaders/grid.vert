#version 450

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec3 v_color;
layout (location = 3) in vec2 v_texcoord;

layout (location = 0) out vec3 near_world_point;
layout (location = 1) out vec3 far_world_point;

layout(set = 0, binding = 0) uniform GpuSceneData {
    mat4 viewproj;
    float near;
    float far;
    vec4 ambient_color;
    vec4 sunlight_direction;
    vec4 sunlight_color;
} scene;

vec3 clip_to_world(vec3 clip_pos) {
    mat4 viewproj_inv = inverse(scene.viewproj);
    vec4 world_pos = viewproj_inv * vec4(clip_pos, 1.0);
    world_pos /= world_pos.w; // Undo perspective projection
    return world_pos.xyz;
}

void main() {
    vec3 clip_pos = v_position;
    // Get the world space position on the near plane
    near_world_point = clip_to_world(vec3(clip_pos.xy, 0.0));
    // Get the world space position on the far plane
    far_world_point = clip_to_world(vec3(clip_pos.xy, 1.0));

    gl_Position = vec4(clip_pos, 1.0);
}
