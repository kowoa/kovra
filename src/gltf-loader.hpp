#pragma once

#include "mesh.hpp"
#include <filesystem>
#include <unordered_map>

namespace kovra {
struct GeometrySurface
{
    uint32_t start_index;
    uint32_t count;
};

struct MeshAsset
{
    std::string name;
    std::vector<GeometrySurface> surfaces;
    Mesh mesh;
};
}
