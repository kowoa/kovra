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
class LoadedGltfScene;

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
    std::optional<std::unique_ptr<LoadedGltfScene>> load_gltf(
      std::filesystem::path filepath,
      const Device &device,
      const RenderResources &resources
    ) const;
};

class LoadedGltfScene : public IRenderable
{
  public:
    explicit LoadedGltfScene(
      fastgltf::Asset gltf,
      const Device &device,
      const RenderResources &resources
    );
    virtual ~LoadedGltfScene() = default;

    virtual void queue_draw(const glm::mat4 &root_transform, DrawContext &ctx)
      const override;

  private:
    constexpr const static bool USE_NORMALS_AS_COLORS = false;

    // Storage for all the data on a given GLTF file
    std::unordered_map<std::string, std::shared_ptr<MeshAsset>> mesh_assets;
    std::unordered_map<std::string, std::shared_ptr<SceneNode>> scene_nodes;
    std::unordered_map<std::string, std::shared_ptr<GpuImage>> textures;
    std::unordered_map<std::string, std::shared_ptr<MaterialInstance>>
      material_instances;

    // Nodes that don't have a parent, used for iterating through the file in
    // tree order
    std::vector<std::shared_ptr<SceneNode>> root_nodes;

    std::vector<vk::UniqueSampler> samplers;
    std::unique_ptr<DescriptorAllocator> desc_alloc;
    // Stores GpuPbrMaterialData
    std::unique_ptr<GpuBuffer> material_buffer;

    static std::optional<std::unique_ptr<GpuImage>> load_image(
      const fastgltf::Asset &asset,
      const fastgltf::Image &image,
      const Device &device,
      const RenderResources &resources
    );
};
}
