#pragma once

#include "asset_loader.hpp"

namespace kovra {
// Forward declarations
class GpuImage;
struct StbImageDeleter;

struct CubemapCreateInfo
{
    std::unique_ptr<unsigned char[], StbImageDeleter> front;
    std::unique_ptr<unsigned char[], StbImageDeleter> back;
    std::unique_ptr<unsigned char[], StbImageDeleter> up;
    std::unique_ptr<unsigned char[], StbImageDeleter> down;
    std::unique_ptr<unsigned char[], StbImageDeleter> right;
    std::unique_ptr<unsigned char[], StbImageDeleter> left;
    int width;
    int height;
    int channels = 4;
};

class Cubemap
{
  public:
    // Use the AssetLoader to get a pointer to the cubemap image data
    Cubemap(CubemapCreateInfo &&ci, const Device &device);

    Cubemap() = delete;
    Cubemap(const Cubemap &) = delete;
    Cubemap(Cubemap &&) = delete;
    Cubemap &operator=(const Cubemap &) = delete;
    Cubemap &operator=(Cubemap &&) = delete;

    [[nodiscard]] const GpuImage &get_image() const noexcept
    {
        return *cubemap;
    }
    [[nodiscard]] GpuImage &get_image_mut() noexcept { return *cubemap; }

  private:
    std::unique_ptr<GpuImage> cubemap;
};
}
