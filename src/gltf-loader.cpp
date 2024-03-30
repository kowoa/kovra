#include "gltf-loader.hpp"

#include "renderer.hpp"

#include "fastgltf/core.hpp"
#include "fastgltf/tools.hpp"
#include "spdlog/spdlog.h"
#include "stb_image.h"

namespace kovra {
std::optional<std::vector<std::shared_ptr<MeshAsset>>>
load_gltf_meshes(Renderer &renderer, std::filesystem::path filepath)
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

    std::vector<std::shared_ptr<MeshAsset>> mesh_assets;

    std::vector<uint32_t> indices;
    std::vector<Vertex> vertices;
    for (fastgltf::Mesh &mesh : gltf.meshes) {
        MeshAsset mesh_asset;
        mesh_asset.name = mesh.name;

        vertices.clear();
        indices.clear();

        for (auto &&p : mesh.primitives) {
            auto surface =
              GeometrySurface{ .start_index =
                                 static_cast<uint32_t>(indices.size()),
                               .count = static_cast<uint32_t>(
                                 gltf.accessors[p.indicesAccessor.value()].count
                               ) };
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
                vertices.reserve(vertices.size() + accessor.count);
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
            }

            mesh_asset.surfaces.push_back(std::move(surface));
        }

        // Display the vertex normals
        constexpr bool override_colors = true;
        if (override_colors) {
            for (Vertex &v : vertices) {
                v.color = glm::vec4(v.normal, 1.0f);
            }
        }

        mesh_asset.mesh = std::make_unique<Mesh>(
          vertices, indices, renderer.get_context().get_device()
        );

        mesh_assets.emplace_back(
          std::make_shared<MeshAsset>(std::move(mesh_asset))
        );
    }

    return mesh_assets;
}
} // namespace kovra
