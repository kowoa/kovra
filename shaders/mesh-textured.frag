#version 450

layout (location = 0) in vec3 in_color;
layout (location = 1) in vec2 in_uv;
layout (location = 0) out vec4 out_color;

layout (set = 1, binding = 0) uniform sampler2D tex_sampler;

void main()
{
    out_color = texture(tex_sampler, in_uv);
}
