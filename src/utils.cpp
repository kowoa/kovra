#include "utils.hpp"

namespace kovra {
namespace utils {
void
transition_image_layout(
  vk::CommandBuffer cmd,
  vk::Image image,
  vk::ImageAspectFlags aspect,
  vk::ImageLayout old_layout,
  vk::ImageLayout new_layout,
  int layer_count
)
{
    if (old_layout == new_layout) {
        return;
    }

    auto image_barrier =
      vk::ImageMemoryBarrier2{}
        .setSrcStageMask(vk::PipelineStageFlagBits2::eAllCommands)
        .setSrcAccessMask(vk::AccessFlagBits2::eMemoryWrite)
        .setDstStageMask(vk::PipelineStageFlagBits2::eAllCommands)
        .setDstAccessMask(
          vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eMemoryRead
        )
        .setOldLayout(old_layout)
        .setNewLayout(new_layout)
        .setSubresourceRange(vk::ImageSubresourceRange{}
                               .setAspectMask(aspect)
                               .setBaseMipLevel(0)
                               .setLevelCount(1)
                               .setBaseArrayLayer(0)
                               .setLayerCount(layer_count))
        .setImage(image);

    auto dep_info = vk::DependencyInfo{}.setImageMemoryBarriers(image_barrier);

    cmd.pipelineBarrier2(dep_info);
}
void
copy_image_to_image(
  vk::CommandBuffer cmd,
  vk::Image src,
  vk::Image dst,
  vk::Extent2D src_size,
  vk::Extent2D dst_size
)
{
    auto blit_region =
      vk::ImageBlit2{}
        .setSrcOffsets({ vk::Offset3D{},
                         vk::Offset3D{ static_cast<int32_t>(src_size.width),
                                       static_cast<int32_t>(src_size.height),
                                       1 } })
        .setDstOffsets({ vk::Offset3D{},
                         vk::Offset3D{ static_cast<int32_t>(dst_size.width),
                                       static_cast<int32_t>(dst_size.height),
                                       1 } })
        .setSrcSubresource(vk::ImageSubresourceLayers{}
                             .setAspectMask(vk::ImageAspectFlagBits::eColor)
                             .setBaseArrayLayer(0)
                             .setLayerCount(1)
                             .setMipLevel(0))
        .setDstSubresource(vk::ImageSubresourceLayers{}
                             .setAspectMask(vk::ImageAspectFlagBits::eColor)
                             .setBaseArrayLayer(0)
                             .setLayerCount(1)
                             .setMipLevel(0));

    auto blit_info = vk::BlitImageInfo2{}
                       .setFilter(vk::Filter::eLinear)
                       .setRegions(blit_region)
                       .setSrcImage(src)
                       .setDstImage(dst)
                       .setSrcImageLayout(vk::ImageLayout::eTransferSrcOptimal)
                       .setDstImageLayout(vk::ImageLayout::eTransferDstOptimal);

    cmd.blitImage2(blit_info);
}
} // namespace utils
} // namespace kovra
