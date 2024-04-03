#pragma once

#include "descriptor.hpp"
#include "render_object.hpp"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "fastgltf/types.hpp"

namespace kovra {
// Forward declarations
class Mesh;
class Renderer;
class LoadedGltfNode;

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
    std::optional<LoadedGltfNode> load_gltf(
      std::filesystem::path filepath,
      const Device &device,
      const RenderResources &resources
    ) const;
};

class LoadedGltfNode : public IRenderable
{
  public:
    explicit LoadedGltfNode(
      fastgltf::Asset gltf,
      const Device &device,
      const RenderResources &resources
    );
    virtual ~LoadedGltfNode() = default;

    virtual void queue_draw(const glm::mat4 &root_transform, DrawContext &ctx)
      const override;

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

    std::vector<vk::UniqueSampler> samplers;
    DescriptorAllocator desc_alloc;
    // Stores GpuPbrMaterialData
    std::unique_ptr<GpuBuffer> material_buffer;
};
}
