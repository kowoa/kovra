#include "asset_loader.hpp"

#include "buffer.hpp"
#include "descriptor.hpp"
#include "mesh.hpp"
#include "pbr_material.hpp"
#include "render_resources.hpp"
#include "renderer.hpp"
#include "vertex.hpp"

#include "fastgltf/core.hpp"
#include "fastgltf/glm_element_traits.hpp"
#include "fastgltf/tools.hpp"
#include "spdlog/spdlog.h"
#include "stb_image.h"

namespace kovra {

vk::Filter
extract_filter(fastgltf::Filter filter)
{
    switch (filter) {
        // Nearest samplers
        case fastgltf::Filter::Nearest:
        case fastgltf::Filter::NearestMipMapNearest:
        case fastgltf::Filter::NearestMipMapLinear:
            return vk::Filter::eNearest;

        // Linear samplers
        case fastgltf::Filter::Linear:
        case fastgltf::Filter::LinearMipMapNearest:
        case fastgltf::Filter::LinearMipMapLinear:
            return vk::Filter::eLinear;

        default:
            return vk::Filter::eNearest;
    }
}

vk::SamplerMipmapMode
extract_mipmap_mode(fastgltf::Filter filter)
{
    switch (filter) {
        case fastgltf::Filter::NearestMipMapNearest:
        case fastgltf::Filter::LinearMipMapNearest:
            return vk::SamplerMipmapMode::eNearest;

        case fastgltf::Filter::NearestMipMapLinear:
        case fastgltf::Filter::LinearMipMapLinear:
            return vk::SamplerMipmapMode::eLinear;

        default:
            return vk::SamplerMipmapMode::eNearest;
    }
}

std::optional<LoadedGltfNode>
AssetLoader::load_gltf(
  std::filesystem::path filepath,
  const Device &device,
  const RenderResources &resources
) const
{
    spdlog::debug("Loading GLTF file: {}", filepath.string());

    // Load the glTF file data into buffer
    fastgltf::GltfDataBuffer data;
    data.loadFromFile(filepath);

    fastgltf::Parser parser{};
    constexpr auto parse_opts = fastgltf::Options::DontRequireValidAssetMember |
                                fastgltf::Options::AllowDouble |
                                fastgltf::Options::LoadGLBBuffers |
                                fastgltf::Options::LoadExternalBuffers;

    // Parse the glTF file
    auto parse_result =
      parser.loadGltf(&data, filepath.parent_path(), parse_opts);
    if (auto error = parse_result.error(); error != fastgltf::Error::None) {
        spdlog::error(
          "Failed to load GLTF file: {}; ERROR: {}",
          filepath.string(),
          fastgltf::to_underlying(error)
        );
        return std::nullopt;
    }
    fastgltf::Asset gltf;
    gltf = std::move(parse_result.get());

    return std::make_optional<LoadedGltfNode>(
      std::move(gltf), device, resources
    );
}

LoadedGltfNode::LoadedGltfNode(
  fastgltf::Asset gltf,
  const Device &device,
  const RenderResources &resources
)
  : desc_alloc{ device.get(),
                static_cast<uint32_t>(gltf.materials.size()),
                { DescriptorPoolSizeRatio{ vk::DescriptorType::eUniformBuffer,
                                           3.0f },
                  DescriptorPoolSizeRatio{ vk::DescriptorType::eStorageBuffer,
                                           1.0f },
                  DescriptorPoolSizeRatio{
                    vk::DescriptorType::eCombinedImageSampler,
                    3.0f } } }
  , material_buffer{ device.create_buffer(
      sizeof(GpuPbrMaterialData) * gltf.materials.size(),
      vk::BufferUsageFlagBits::eUniformBuffer,
      VMA_MEMORY_USAGE_CPU_TO_GPU,
      VMA_ALLOCATION_CREATE_MAPPED_BIT
    ) }
{
    for (const fastgltf::Sampler &sampler : gltf.samplers) {
        auto vk_sampler_ci =
          vk::SamplerCreateInfo{}
            .setMaxLod(vk::LodClampNone)
            .setMinLod(0.0f)
            .setMagFilter(extract_filter(
              sampler.magFilter.value_or(fastgltf::Filter::Nearest)
            ))
            .setMinFilter(extract_filter(
              sampler.minFilter.value_or(fastgltf::Filter::Nearest)
            ));
        samplers.emplace_back(device.get().createSamplerUnique(vk_sampler_ci));
    }

    // Load textures
    std::vector<std::shared_ptr<GpuImage>> textures;
    for (const fastgltf::Image &img : gltf.images) {
        textures.push_back(resources.get_texture_owned("checkboard"));
    }

    // Load materials
    std::vector<std::shared_ptr<MaterialInstance>> material_instances;
    for (size_t i = 0; i < gltf.materials.size(); i++) {
        const fastgltf::Material &mat = gltf.materials[i];
        const auto material_data =
          GpuPbrMaterialData{ .color_factors = glm::vec4(
                                mat.pbrData.baseColorFactor[0],
                                mat.pbrData.baseColorFactor[1],
                                mat.pbrData.baseColorFactor[2],
                                mat.pbrData.baseColorFactor[3]
                              ),
                              .metal_rough_factors = glm::vec4(
                                mat.pbrData.metallicFactor,
                                mat.pbrData.roughnessFactor,
                                0.0f,
                                0.0f
                              ),
                              ._padding = {} };
        material_buffer->write(
          &material_data,
          sizeof(GpuPbrMaterialData),
          i * sizeof(GpuPbrMaterialData)
        );

        MaterialPass pass = mat.alphaMode == fastgltf::AlphaMode::Blend
                              ? MaterialPass::Transparent
                              : MaterialPass::Opaque;

        // Grab textures from glTF file
        const GpuImage *albedo_texture;
        const vk::Sampler *albedo_sampler;
        if (mat.pbrData.baseColorTexture.has_value()) {
            size_t img_idx =
              gltf.textures[mat.pbrData.baseColorTexture.value().textureIndex]
                .imageIndex.value();
            size_t sampler_idx =
              gltf.textures[mat.pbrData.baseColorTexture.value().textureIndex]
                .samplerIndex.value();

            albedo_texture = &*textures[img_idx];
            albedo_sampler = &samplers[sampler_idx].get();
        } else {
            albedo_texture = &resources.get_texture("white");
            albedo_sampler = &resources.get_sampler(vk::Filter::eLinear);
        }

        auto mat_inst_ci = PbrMaterialInstanceCreateInfo{
            .albedo_texture = *albedo_texture,
            .albedo_sampler = *albedo_sampler,
            .metal_rough_texture = resources.get_texture("white"),
            .metal_rough_sampler = resources.get_sampler(vk::Filter::eLinear),
            .material_buffer = material_buffer->get(),
            .material_buffer_offset =
              static_cast<uint32_t>(i * sizeof(GpuPbrMaterialData)),
            .pass = pass,
        };
        auto material_instance =
          resources.get_pbr_material().create_material_instance(
            mat_inst_ci, device, desc_alloc
          );
    }

    // Load meshes
}

/*
std::optional<std::vector<MeshAsset>>
AssetLoader::load_gltf_meshes(
  const Renderer &renderer,
  std::filesystem::path filepath
) const
{
    spdlog::debug("Loading GLTF file: {}", filepath.string());

    // Load the GLTF file data into buffer
    fastgltf::GltfDataBuffer data;
    data.loadFromFile(filepath);

    // Parse the GLTF file
    fastgltf::Parser parser{};
    constexpr auto parse_opts = fastgltf::Options::LoadGLBBuffers |
                                fastgltf::Options::LoadExternalBuffers;
    auto result =
      parser.loadGltfBinary(&data, filepath.parent_path(), parse_opts);
    if (auto error = result.error(); error != fastgltf::Error::None) {
        spdlog::error("Failed to load GLTF file: {}", filepath.string());
        return std::nullopt;
    }

    fastgltf::Asset gltf;
    gltf = std::move(result.get());

    std::vector<MeshAsset> mesh_assets;

    std::vector<uint32_t> indices;
    std::vector<Vertex> vertices;
    for (fastgltf::Mesh &mesh : gltf.meshes) {
        MeshAsset mesh_asset;
        mesh_asset.name = mesh.name;

        vertices.clear();
        indices.clear();

        for (auto &&p : mesh.primitives) {
            auto surface = MeshAsset::GeometrySurface{
                .start_index = static_cast<uint32_t>(indices.size()),
                .count = static_cast<uint32_t>(
                  gltf.accessors[p.indicesAccessor.value()].count
                )
            };
            size_t initial_vertex_count = vertices.size();

            // Load indices
            {
                fastgltf::Accessor &accessor =
                  gltf.accessors[p.indicesAccessor.value()];
                indices.reserve(indices.size() + accessor.count);
                fastgltf::iterateAccessor<uint32_t>(
                  gltf,
                  accessor,
                  [&](uint32_t idx) {
                      indices.push_back(initial_vertex_count + idx);
                  }
                );
            }

            // Load vertex positions
            {
                fastgltf::Accessor &accessor =
                  gltf.accessors[p.findAttribute("POSITION")->second];
                vertices.resize(vertices.size() + accessor.count);
                fastgltf::iterateAccessorWithIndex<glm::vec3>(
                  gltf,
                  accessor,
                  [&](glm::vec3 pos, size_t idx) {
                      auto vertex = Vertex{
                          .position = pos,
                          .normal = glm::vec3(0.0f, 1.0f, 0.0f),
                          .color = glm::vec3(1.0f),
                          .uv = glm::vec2(0.0f),
                      };
                      vertices[initial_vertex_count + idx] =
std::move(vertex);
                  }
                );
            }

            // Load vertex normals
            if (auto normals = p.findAttribute("NORMAL");
                normals != p.attributes.end()) {
                fastgltf::iterateAccessorWithIndex<glm::vec3>(
                  gltf,
                  gltf.accessors[(*normals).second],
                  [&](glm::vec3 normal, size_t idx) {
                      vertices[initial_vertex_count + idx].normal = normal;
                  }
                );
            } else {
                spdlog::warn(
                  "No vertex normals found in mesh: {}", mesh_asset.name
                );
            }

            // Load UVs
            if (auto uvs = p.findAttribute("TEXCOORD_0");
                uvs != p.attributes.end()) {
                fastgltf::iterateAccessorWithIndex<glm::vec2>(
                  gltf,
                  gltf.accessors[(*uvs).second],
                  [&](glm::vec2 uv, size_t idx) {
                      vertices[initial_vertex_count + idx].uv = uv;
                  }
                );
            } else {
                spdlog::warn("No UVs found in mesh: {}", mesh_asset.name);
            }

            // Load vertex colors
            if (auto colors = p.findAttribute("COLOR_0");
                colors != p.attributes.end()) {
                fastgltf::iterateAccessorWithIndex<glm::vec4>(
                  gltf,
                  gltf.accessors[(*colors).second],
                  [&](glm::vec4 color, size_t idx) {
                      vertices[initial_vertex_count + idx].color = color;
                  }
                );
            } else {
                spdlog::warn(
                  "No vertex colors found in mesh: {}", mesh_asset.name
                );
            }

            mesh_asset.surfaces.push_back(std::move(surface));
        }

        // Display the vertex normals
        constexpr bool override_colors = false;
        if (override_colors) {
            for (Vertex &v : vertices) {
                v.color = glm::vec4(v.normal, 1.0f);
            }
        }

        mesh_asset.mesh = std::make_unique<Mesh>(
          vertices, indices, renderer.get_context().get_device()
        );

        mesh_assets.emplace_back(std::move(mesh_asset));
    }

    return mesh_assets;
}
*/
} // namespace kovra
