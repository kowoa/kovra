#pragma once

#include "render_object.hpp"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace kovra {
// Forward declarations
class Mesh;
class Renderer;
class DescriptorAllocator;

struct MeshAsset
{
    struct GeometrySurface
    {
        uint32_t start_index;
        uint32_t count;
        std::shared_ptr<MaterialInstance> material_instance;
    };

    std::string name;
    std::vector<GeometrySurface> surfaces;
    std::unique_ptr<Mesh> mesh;
};

class AssetLoader
{
  public:
    std::optional<std::vector<MeshAsset>> load_gltf_meshes(
      const Renderer &renderer,
      std::filesystem::path filepath
    ) const;
};

class LoadedGltfNode : public IRenderable
{
  private:
    // Storage for all the data on a given GLTF file
    std::unordered_map<std::string, std::shared_ptr<MeshAsset>> mesh_assets;
    std::unordered_map<std::string, std::shared_ptr<SceneNode>> scene_nodes;
    std::unordered_map<std::string, GpuImage> textures;
    std::unordered_map<std::string, std::shared_ptr<MaterialInstance>>
      material_instances;

    // Nodes that don't have a parent, used for iterating through the file in
    // tree order
    std::vector<std::shared_ptr<SceneNode>> root_nodes;

    std::vector<vk::Sampler> samplers;
    std::unique_ptr<DescriptorAllocator> desc_alloc;
    // Stores GpuPbrMaterialData
    std::unique_ptr<GpuBuffer> material_buffer;
};
}
