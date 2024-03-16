#pragma once

#include <vulkan/vulkan.hpp>

namespace kovra {
static constexpr const char *SHADERBUILD_DIR = "./shaderbuild";

class GraphicsShader {
  public:
    GraphicsShader(const std::string &name, const vk::Device &device);

    [[nodiscard]] vk::ShaderModule get_vert_shader_mod() const {
        return vert_shader_mod.get();
    }
    [[nodiscard]] vk::ShaderModule get_frag_shader_mod() const {
        return frag_shader_mod.get();
    }

  private:
    vk::UniqueShaderModule vert_shader_mod;
    vk::UniqueShaderModule frag_shader_mod;
};

class ComputeShader {
  public:
    ComputeShader(const std::string &name, const vk::Device &device);
    [[nodiscard]] vk::ShaderModule get_shader_mod() const {
        return shader_mod.get();
    }

  private:
    vk::UniqueShaderModule shader_mod;
};
} // namespace kovra
