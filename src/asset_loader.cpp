#include "asset_loader.hpp"

#include "buffer.hpp"
#include "descriptor.hpp"
#include "mesh.hpp"
#include "pbr_material.hpp"
#include "render_object.hpp"
#include "render_resources.hpp"
#include "renderer.hpp"
#include "vertex.hpp"

#include "fastgltf/core.hpp"
#include "fastgltf/glm_element_traits.hpp"
#include "fastgltf/tools.hpp"
#include "glm/gtc/quaternion.hpp"
#include "spdlog/spdlog.h"

#define STB_IMAGE_IMPLEMENTATION
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

std::optional<std::unique_ptr<LoadedGltfScene>>
AssetLoader::load_gltf(
  std::filesystem::path filepath,
  const Device &device,
  const RenderResources &resources
)
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

    return std::make_unique<LoadedGltfScene>(
      std::move(gltf), device, resources
    );
}

std::optional<std::unique_ptr<unsigned char[], StbImageDeleter>>
AssetLoader::load_image_raw(
  const std::filesystem::path &filepath,
  int *width,
  int *height,
  int *channels
)
{
    int img_width, img_height, img_channels;
    unsigned char *data = stbi_load(
      filepath.c_str(), &img_width, &img_height, &img_channels, STBI_rgb_alpha
    );

    if (width != nullptr) {
        *width = img_width;
    }
    if (height != nullptr) {
        *height = img_height;
    }
    if (channels != nullptr) {
        *channels = img_channels;
    }

    if (data) {
        return std::unique_ptr<unsigned char[], StbImageDeleter>(data);
    }
    return std::nullopt;
}

std::optional<std::unique_ptr<GpuImage>>
LoadedGltfScene::load_image(
  const fastgltf::Asset &asset,
  const fastgltf::Image &image,
  const Device &device,
  const RenderResources &resources
)
{
    std::unique_ptr<GpuImage> img = nullptr;
    int width, height, channels;

    std::visit(
      fastgltf::visitor{
        [](auto &arg) {
            spdlog::error("Unknown image data type: {}", typeid(arg).name());
        },
        [&](const fastgltf::sources::URI &filepath) {
            assert(
              filepath.fileByteOffset == 0
            ); // We don't support offsets with stbi
            assert(filepath.uri.isLocalPath()); // We only support local paths

            const std::string filepath_str{ filepath.uri.path().begin(),
                                            filepath.uri.path().end() };
            spdlog::debug("Loading image: {}", filepath_str);
            auto result = AssetLoader::load_image_raw(
              filepath_str, &width, &height, &channels
            );
            if (result.has_value()) {
                unsigned char *data = result.value().get();
                img = device.create_color_image(
                  data,
                  width,
                  height,
                  resources.get_sampler(vk::Filter::eLinear),
                  vk::Format::eR8G8B8A8Unorm
                );
                result.reset();
            }
        },
        [&](const fastgltf::sources::Vector &vector) {
            unsigned char *data = stbi_load_from_memory(
              vector.bytes.data(),
              static_cast<int>(vector.bytes.size()),
              &width,
              &height,
              &channels,
              4
            );
            if (data) {
                img = device.create_color_image(
                  data,
                  width,
                  height,
                  resources.get_sampler(vk::Filter::eLinear),
                  vk::Format::eR8G8B8A8Unorm,
                  true
                );
                stbi_image_free(data);
            }
        },
        [&](const fastgltf::sources::BufferView &view) {
            const auto &buffer_view = asset.bufferViews[view.bufferViewIndex];
            const auto &buffer = asset.buffers[buffer_view.bufferIndex];

            std::visit(
              fastgltf::visitor{
                [](auto &arg) {
                    spdlog::error(
                      "Unknown buffer data type: {}", typeid(arg).name()
                    );
                },
                [&](const fastgltf::sources::Array &vector) {
                    unsigned char *data = stbi_load_from_memory(
                      vector.bytes.data() + buffer_view.byteOffset,
                      static_cast<int>(buffer_view.byteLength),
                      &width,
                      &height,
                      &channels,
                      4
                    );
                    if (data) {
                        img = device.create_color_image(
                          data,
                          width,
                          height,
                          resources.get_sampler(vk::Filter::eLinear),
                          vk::Format::eR8G8B8A8Unorm
                        );
                        stbi_image_free(data);
                    }
                } },
              buffer.data
            );
        } },
      image.data
    );

    if (!img) {
        return std::nullopt;
    } else {
        return img;
    }
}

LoadedGltfScene::LoadedGltfScene(
  fastgltf::Asset gltf,
  const Device &device,
  const RenderResources &resources
)
  : desc_alloc{ std::make_unique<DescriptorAllocator>(
      device.get(),
      static_cast<uint32_t>(gltf.materials.size()),
      std::vector<DescriptorPoolSizeRatio>{
        DescriptorPoolSizeRatio{ vk::DescriptorType::eUniformBuffer, 3.0f },
        DescriptorPoolSizeRatio{ vk::DescriptorType::eStorageBuffer, 1.0f },
        DescriptorPoolSizeRatio{ vk::DescriptorType::eCombinedImageSampler,
                                 3.0f } }
    ) }
  , material_buffer{ device.create_buffer(
      sizeof(GpuPbrMaterialData) * gltf.materials.size(),
      vk::BufferUsageFlagBits::eUniformBuffer,
      VMA_MEMORY_USAGE_CPU_TO_GPU,
      VMA_ALLOCATION_CREATE_MAPPED_BIT
    ) }
{
    // Load samplers
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
            ))
            .setMipmapMode(extract_mipmap_mode(
              sampler.minFilter.value_or(fastgltf::Filter::Nearest)
            ));
        samplers.emplace_back(device.get().createSamplerUnique(vk_sampler_ci));
    }

    // Load textures
    std::vector<std::shared_ptr<GpuImage>> textures;
    for (const fastgltf::Image &img : gltf.images) {
        auto result = load_image(gltf, img, device, resources);

        std::shared_ptr<GpuImage> texture = nullptr;
        if (result.has_value()) {
            texture = std::move(result.value());
        } else {
            texture = resources.get_texture_owned("checkerboard");
            spdlog::error("Failed to load image: {}", img.name.c_str());
        }

        textures.push_back(texture);
        this->textures[img.name.c_str()] = texture;
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
        auto material_instance = std::make_shared<MaterialInstance>(
          resources.get_pbr_material().create_material_instance(
            mat_inst_ci, device, *desc_alloc
          )
        );
        material_instances.push_back(material_instance);
        this->material_instances[mat.name.c_str()] = material_instance;
    }

    // Load meshes
    std::vector<std::shared_ptr<MeshAsset>> mesh_assets;
    std::vector<uint32_t> indices;
    std::vector<Vertex> vertices;
    for (fastgltf::Mesh &mesh : gltf.meshes) {
        auto mesh_asset = std::make_shared<MeshAsset>();
        mesh_asset->name = mesh.name;

        vertices.clear();
        indices.clear();

        for (auto &&p : mesh.primitives) {
            auto surface = GeometrySurface{
                .start_index = static_cast<uint32_t>(indices.size()),
                .count = static_cast<uint32_t>(
                  gltf.accessors[p.indicesAccessor.value()].count
                ),
                .bounds = {},
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
                      vertices[initial_vertex_count + idx] = std::move(vertex);
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
                  "No vertex normals found in mesh: {}", mesh_asset->name
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
                spdlog::warn("No UVs found in mesh: {}", mesh_asset->name);
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
                  "No vertex colors found in mesh: {}", mesh_asset->name
                );
            }

            // Load material
            if (p.materialIndex.has_value()) {
                surface.material_instance =
                  material_instances[p.materialIndex.value()];
            } else {
                surface.material_instance = material_instances[0];
            }

            // Set bounds
            {
                glm::vec3 min_pos = vertices[initial_vertex_count].position;
                glm::vec3 max_pos = vertices[initial_vertex_count].position;
                // Loop through vertices of surface to find the min and max
                // bounds
                for (size_t i = initial_vertex_count; i < vertices.size();
                     i++) {
                    min_pos = glm::min(min_pos, vertices[i].position);
                    max_pos = glm::max(max_pos, vertices[i].position);
                }
                // Calculate the origin and extents from the min/max
                surface.bounds.origin = (min_pos + max_pos) / 2.0f;
                surface.bounds.extents = (max_pos - min_pos) / 2.0f;
                // Use entents length as sphere radius
                surface.bounds.sphere_radius =
                  glm::length(surface.bounds.extents);
            }

            mesh_asset->surfaces.push_back(std::move(surface));
        }

        // Override vertex colors with normals
        if (USE_NORMALS_AS_COLORS) {
            for (Vertex &v : vertices) {
                v.color = glm::vec4(v.normal, 1.0f);
            }
        }

        mesh_asset->mesh = std::make_unique<Mesh>(vertices, indices, device);

        mesh_assets.push_back(mesh_asset);
        this->mesh_assets[mesh.name.c_str()] = mesh_asset;
    }

    // Load scene nodes
    std::vector<std::shared_ptr<SceneNode>> scene_nodes;
    for (const fastgltf::Node &node : gltf.nodes) {
        std::shared_ptr<SceneNode> scene_node = nullptr;
        if (node.meshIndex.has_value()) {
            scene_node =
              std::make_shared<MeshNode>(mesh_assets[node.meshIndex.value()]);
        } else {
            scene_node = std::make_shared<SceneNode>();
        }

        std::visit(
          fastgltf::visitor{
            [&](fastgltf::Node::TransformMatrix loc_tf) {
                glm::mat4 local_transform;
                memcpy(&local_transform, loc_tf.data(), sizeof(loc_tf));
                scene_node->set_local_transform(local_transform);
            },
            [&](fastgltf::TRS loc_tf) {
                glm::vec3 t{ loc_tf.translation[0],
                             loc_tf.translation[1],
                             loc_tf.translation[2] };
                glm::quat r{ loc_tf.rotation[3],
                             loc_tf.rotation[0],
                             loc_tf.rotation[1],
                             loc_tf.rotation[2] };
                glm::vec3 s{ loc_tf.scale[0],
                             loc_tf.scale[1],
                             loc_tf.scale[2] };
                glm::mat4 tm = glm::translate(glm::identity<glm::mat4>(), t);
                glm::mat4 rm = glm::mat4_cast(r);
                glm::mat4 sm = glm::scale(glm::identity<glm::mat4>(), s);
                scene_node->set_local_transform(tm * rm * sm);
            } },
          node.transform
        );

        scene_nodes.push_back(scene_node);
        this->scene_nodes[node.name.c_str()] = scene_node;
    }

    // Run loop again to set up parent-child relationships
    for (size_t i = 0; i < gltf.nodes.size(); i++) {
        const fastgltf::Node &node = gltf.nodes[i];
        std::shared_ptr<SceneNode> scene_node = scene_nodes[i];
        for (const auto &child : node.children) {
            scene_node->add_child(scene_nodes[child]);
        }
    }

    // Find the root nodes
    for (const auto &scene_node : scene_nodes) {
        if (!scene_node->has_parent()) {
            root_nodes.push_back(scene_node);
            scene_node->refresh_world_transform(glm::identity<glm::mat4>());
        }
    }
}

void
LoadedGltfScene::queue_draw(const glm::mat4 &root_transform, DrawContext &ctx)
  const
{
    for (const auto &node : root_nodes) {
        node->queue_draw(root_transform, ctx);
    }
}

} // namespace kovra
