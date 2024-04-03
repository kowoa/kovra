#version 450
#extension GL_GOOGLE_include_directive: require
#extension GL_EXT_buffer_reference : require

#include "input_structures.glsl"

layout (location = 0) out vec3 out_normal;
layout (location = 1) out vec3 out_color;
layout (location = 2) out vec2 out_uv;

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

layout (push_constant) uniform GpuPushConstants {
    mat4 object_transform;
    VertexBuffer vertex_buffer;
} PushConstants;

void main() {
    Vertex v = PushConstants.vertex_buffer.vertices[gl_VertexIndex];
    gl_Position = Scene.viewproj * PushConstants.object_transform * vec4(v.position, 1.0);

    out_normal = (PushConstants.object_transform * vec4(v.normal, 0.0f)).xyz;
    out_normal = normalize(out_normal);

    out_color = v.color.xyz * Material.color_factors.xyz;
    out_uv = vec2(v.uv_x, v.uv_y);
}
