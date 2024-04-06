#version 450

layout (location = 0) out vec3 out_uv;

layout (push_constant) uniform GpuPushConstants {
    mat4 viewproj;
    mat4 camera_translation;
} PushConstants;

const vec3 vertices[8] = vec3[](
    vec3(-1.0,  1.0, -1.0), // Top front left
    vec3(-1.0, -1.0, -1.0), // Bottom front left
    vec3( 1.0, -1.0, -1.0), // Bottom front right
    vec3( 1.0,  1.0, -1.0), // Top front right
    vec3(-1.0,  1.0,  1.0), // Top back left
    vec3(-1.0, -1.0,  1.0), // Bottom back left
    vec3( 1.0, -1.0,  1.0), // Bottom back right
    vec3( 1.0,  1.0,  1.0)  // Top back right
);

const int indices[36] = int[](
    0, 1, 2, 2, 3, 0, // Front
    4, 7, 6, 6, 5, 4, // Back
    0, 4, 5, 5, 1, 0, // Top
    3, 2, 6, 6, 7, 3, // Bottom
    0, 3, 7, 7, 4, 0, // Right
    1, 5, 6, 6, 2, 1  // Left
);

void main() {
    int index = indices[gl_VertexIndex];
    vec3 vertex = vertices[index];

    out_uv = vertex;
    gl_Position = PushConstants.viewproj * PushConstants.camera_translation * vec4(vertex, 1.0);
}
