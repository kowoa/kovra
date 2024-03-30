#pragma once

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

struct MeshAsset
{
    struct GeometrySurface
    {
        uint32_t start_index;
        uint32_t count;
    };

    std::string name;
    std::vector<GeometrySurface> surfaces;
    std::unique_ptr<Mesh> mesh;
};

class AssetLoader
{
  public:
    std::optional<std::vector<std::shared_ptr<MeshAsset>>> load_gltf_meshes(
      const Renderer &renderer,
      std::filesystem::path filepath
    ) const;
};
}
