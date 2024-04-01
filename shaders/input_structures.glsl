layout (set = 0, binding = 0) uniform GpuSceneData {
    mat4 viewproj;
    float near;
    float far;
    vec4 ambient_color;
    vec4 sunlight_direction; // w for sun power
    vec4 sunlight_color;
} Scene;

layout (set = 1, binding = 0) uniform GpuPbrMaterialData {
    vec4 color_factors;
    vec4 metal_rough_factors;
} Material;
layout (set = 1, binding = 1) uniform sampler2D albedo_tex;
layout (set = 1, binding = 2) uniform sampler2D metal_rough_tex;
