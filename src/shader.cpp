#include "shader.hpp"
#include "spdlog/spdlog.h"
#include <filesystem>
#include <fstream>

namespace kovra {
GraphicsShader::GraphicsShader(
    const std::string &name, const vk::Device &device) {
    // Construct files paths for the vertex and fragment shaders
    std::filesystem::path vert_filepath{SHADERBUILD_DIR};
    vert_filepath.append(name + ".vert.spv");
    std::filesystem::path frag_filepath{SHADERBUILD_DIR};
    frag_filepath.append(name + ".frag.spv");

    // Read vertex shader SPIR-V
    std::vector<char> vert_spv;
    std::ifstream vert_file{vert_filepath, std::ios::binary};
    vert_spv.assign(
        std::istreambuf_iterator<char>(vert_file),
        std::istreambuf_iterator<char>());

    // Read fragment shader SPIR-V
    std::vector<char> frag_spv;
    std::ifstream frag_file{frag_filepath, std::ios::binary};
    frag_spv.assign(
        std::istreambuf_iterator<char>(frag_file),
        std::istreambuf_iterator<char>());

    // Create shader modules
    vert_shader_mod =
        device.createShaderModuleUnique(vk::ShaderModuleCreateInfo(
            vk::ShaderModuleCreateFlags(), vert_spv.size(),
            reinterpret_cast<const uint32_t *>(vert_spv.data())));
    frag_shader_mod =
        device.createShaderModuleUnique(vk::ShaderModuleCreateInfo(
            vk::ShaderModuleCreateFlags(), frag_spv.size(),
            reinterpret_cast<const uint32_t *>(frag_spv.data())));
}

ComputeShader::ComputeShader(
    const std::string &name, const vk::Device &device) {
    // Construct file path for the compute shader
    std::filesystem::path filepath{SHADERBUILD_DIR};
    filepath.append(name + ".comp.spv");

    // Read compute shader SPIR-V
    std::vector<char> spv;
    std::ifstream file{filepath, std::ios::binary};
    spv.assign(
        std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

    // Create shader module
    shader_mod = device.createShaderModuleUnique(vk::ShaderModuleCreateInfo(
        vk::ShaderModuleCreateFlags(), spv.size(),
        reinterpret_cast<const uint32_t *>(spv.data())));
}
} // namespace kovra
