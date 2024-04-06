#version 450

layout (location = 0) in vec3 in_uv;
layout (location = 0) out vec4 out_color;

layout (set = 0, binding = 0) uniform samplerCube skybox_tex;

void main() {
    out_color = texture(skybox_tex, in_uv);
}
