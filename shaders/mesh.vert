#version 450
#extension GL_EXT_buffer_reference : require

layout (location = 0) out vec3 out_color;
layout (location = 1) out vec2 out_uv;

struct Vertex {
    vec3 position;
    float uv_x;
    vec3 normal;
    float uv_y;
    vec4 color;
};

layout (buffer_reference, std430) readonly buffer VertexBuffer {
    Vertex vertices[];
};

layout (set = 0, binding = 0) uniform GpuSceneData {
    mat4 viewproj;
    float near;
    float far;
    vec4 ambient_color;
    vec4 sunlight_direction;
    vec4 sunlight_color;
} Scene;

layout (push_constant) uniform PushConstants {
    VertexBuffer vertex_buffer;
} Constants;

void main() {
    Vertex v = Constants.vertex_buffer.vertices[gl_VertexIndex];
    out_color = v.color.xyz;
    out_uv = vec2(v.uv_x, v.uv_y);
    gl_Position = Scene.viewproj * vec4(v.position, 1.0);
}
