#pragma once

#include <memory>

namespace kovra {
// Forward declarations
class GpuImage;

class Cubemap
{
  public:
    // Use the AssetLoader to get a pointer to the cubemap image
    Cubemap();

  private:
    std::unique_ptr<GpuImage> cubemap;
};
}
