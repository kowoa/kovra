#include "cubemap.hpp"
#include "device.hpp"
#include "image.hpp"

namespace kovra {

Cubemap::Cubemap(CubemapCreateInfo &&ci, const Device &device)
{
    // 4 channels for RGBA
    // 6 faces of the cube
    const vk::DeviceSize single_image_size = ci.width * ci.height * 4;
    const vk::DeviceSize cubemap_image_size = single_image_size * 6;

    // Write image data to a staging buffer
    auto staging_buffer = device.create_buffer(
      cubemap_image_size,
      vk::BufferUsageFlagBits::eTransferSrc,
      VMA_MEMORY_USAGE_CPU_TO_GPU,
      VMA_ALLOCATION_CREATE_MAPPED_BIT
    );
    staging_buffer->write(
      ci.front.get(), single_image_size, single_image_size * 0
    );
    staging_buffer->write(
      ci.back.get(), single_image_size, single_image_size * 1
    );
    staging_buffer->write(
      ci.up.get(), single_image_size, single_image_size * 2
    );
    staging_buffer->write(
      ci.down.get(), single_image_size, single_image_size * 3
    );
    staging_buffer->write(
      ci.left.get(), single_image_size, single_image_size * 4
    );
    staging_buffer->write(
      ci.right.get(), single_image_size, single_image_size * 5
    );

    // Create the cubemap image
    auto img_ci = GpuImageCreateInfo{
        .format = vk::Format::eR8G8B8A8Unorm,
        .extent = vk::Extent3D(ci.width, ci.height, 1),
        .usage = vk::ImageUsageFlagBits::eSampled |
                 vk::ImageUsageFlagBits::eTransferDst,
        .aspect = vk::ImageAspectFlagBits::eColor,
        .view_type = vk::ImageViewType::eCube,
        .mipmapped = false,
        .sampler = std::nullopt,
        .array_layers = 6,
        .flags = vk::ImageCreateFlagBits::eCubeCompatible,
    };
    cubemap = device.create_image(img_ci);
    cubemap->upload(staging_buffer->get(), device, false);
}
}
