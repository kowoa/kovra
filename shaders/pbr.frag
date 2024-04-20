#version 450

#extension GL_GOOGLE_include_directive : require
#include "input_structures.glsl"

layout (location = 0) in vec3 in_normal;
layout (location = 1) in vec3 in_world_pos;
layout (location = 2) in vec2 in_uv;
layout (location = 3) in vec4 in_color;

layout (location = 0) out vec4 out_color;

const float PI = 3.14159265359f;

vec3 fresnel_schlick(float cos_theta, vec3 F0) {
    return F0 + (1.0f - F0) * pow(clamp(1.0f - cos_theta, 0.0f, 1.0f), 5.0f);
}

float distribution_ggx(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0f);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = PI * denom * denom;

    return num / denom;
}

float geometry_schlick_ggx(float NdotV, float roughness) {
    float r = (roughness + 1.0f);
    float k = (r * r) / 8.0f;

    float num = NdotV;
    float denom = NdotV * (1.0f - k) + k;

    return num / denom;
}

float geometry_smith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0f);
    float NdotL = max(dot(N, L), 0.0f);
    float ggx1 = geometry_schlick_ggx(NdotV, roughness);
    float ggx2 = geometry_schlick_ggx(NdotL, roughness);

    return ggx1 * ggx2;
}

const vec3 light_positions[4] = {
    vec3(-5.0f, 5.0f, 10.0f),
    vec3(5.0f, 5.0f, 10.0f),
    vec3(-5.0f, -5.0f, 10.0f),
    vec3(5.0f, -5.0f, 10.0f)
};

const vec3 light_colors[4] = {
    vec3(100.0f),
    vec3(100.0f),
    vec3(100.0f),
    vec3(100.0f)
};

void main()
{
    vec3 albedo = (in_color * texture(albedo_tex, in_uv)).rgb;
    vec4 metallic_roughness = texture(metal_rough_tex, in_uv);
    float metallic = metallic_roughness.b * Material.metal_rough_factors.r;
    float roughness = metallic_roughness.g * Material.metal_rough_factors.g;
    float ambient_occlusion = texture(ambient_occlusion_tex, in_uv).r;

    vec3 N = in_normal;
    vec3 V = normalize(Scene.cam_world_pos.xyz - in_world_pos);

    // Reflectance equation
    vec3 Lo = vec3(0.0f);
    for (int i = 0; i < 4; i++) {
        vec3 L = normalize(light_positions[i] - in_world_pos);
        vec3 H = normalize(V + L);

        // Calculate per-light radiance
        float distance = length(light_positions[i] - in_world_pos);
        float attenuation = 1.0f / (distance * distance);
        vec3 radiance = light_colors[i] * attenuation;

        vec3 F0 = vec3(0.04f);
        F0 = mix(F0, albedo, vec3(metallic));
        vec3 F = fresnel_schlick(max(dot(H, V), 0.0f), F0);

        float NDF = distribution_ggx(N, H, roughness);
        float G = geometry_smith(N, V, L, roughness);

        // Calculate Cook-Torrance BRDF
        vec3 numerator = NDF * G * F;
        float denominator = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f) + 0.0001f;
        vec3 specular = numerator / denominator;

        vec3 kS = F; // Energy of light that is reflected
        vec3 kD = vec3(1.0f) - kS; // Energy of light that is refracted
        kD *= 1.0f - metallic.r; // Energy of light that is absorbed

        // Add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0f);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    vec3 ambient_color = vec3(0.03f) * albedo * ambient_occlusion;
    out_color = vec4(Lo + ambient_color, 1.0f);
    out_color /= out_color + vec4(1.0f);
    out_color = pow(out_color, vec4(1.0f / 2.2f));

  /*
    float light_value = max(dot(in_normal, -Scene.sunlight_direction.xyz), 0.1f);

    vec3 color = in_color * texture(albedo_tex, in_uv).xyz;
    vec3 ambient = color * Scene.ambient_color.xyz;

    out_color = vec4(color * light_value * Scene.sunlight_color.w + ambient, 1.0f);
  */
}
