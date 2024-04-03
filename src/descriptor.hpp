#pragma once

#include <vulkan/vulkan.hpp>

namespace kovra {
class DescriptorSetLayoutBuilder
{
  public:
    [[nodiscard]] DescriptorSetLayoutBuilder &add_binding(
      uint32_t binding,
      vk::DescriptorType descriptor_type,
      vk::ShaderStageFlags stage_flags
    );

    [[nodiscard]] vk::DescriptorSetLayout build(const vk::Device &device) const;
    [[nodiscard]] vk::UniqueDescriptorSetLayout build_unique(
      const vk::Device &device
    ) const;

  private:
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
};

struct DescriptorPoolSizeRatio
{
    vk::DescriptorType desc_type;
    float ratio;
};

class DescriptorAllocator
{
  public:
    explicit DescriptorAllocator(
      const vk::Device &device,
      uint32_t max_sets,
      std::vector<DescriptorPoolSizeRatio> pool_ratios = {
        DescriptorPoolSizeRatio{ vk::DescriptorType::eUniformBuffer, 3.0f },
        DescriptorPoolSizeRatio{ vk::DescriptorType::eStorageBuffer, 3.0f },
        DescriptorPoolSizeRatio{ vk::DescriptorType::eCombinedImageSampler,
                                 4.0f },
        DescriptorPoolSizeRatio{ vk::DescriptorType::eStorageImage, 3.0f },
    }
    );
    ~DescriptorAllocator();
    DescriptorAllocator(const DescriptorAllocator &) = delete;
    DescriptorAllocator &operator=(const DescriptorAllocator &) = delete;
    DescriptorAllocator(DescriptorAllocator &&) = delete;
    DescriptorAllocator &operator=(DescriptorAllocator &&) = delete;

    [[nodiscard]] vk::DescriptorSet
    allocate(vk::DescriptorSetLayout layout, const vk::Device &device);

    void clear_pools(const vk::Device &device);
    void destroy_pools();

  private:
    // Needed to reallocate pools
    std::vector<DescriptorPoolSizeRatio> pool_ratios;
    // Pools that cannot allocate more sets
    std::vector<vk::UniqueDescriptorPool> full_pools;
    // Pools that can allocate more sets
    std::vector<vk::UniqueDescriptorPool> ready_pools;
    uint32_t sets_per_pool;

    vk::UniqueDescriptorPool get_next_ready_pool(const vk::Device &device);
};

class DescriptorWriter
{
  public:
    DescriptorWriter() = default;
    ~DescriptorWriter() = default;
    DescriptorWriter(const DescriptorWriter &) = delete;
    DescriptorWriter &operator=(const DescriptorWriter &) = delete;
    DescriptorWriter(DescriptorWriter &&) = delete;
    DescriptorWriter &operator=(DescriptorWriter &&) = delete;

    void write_buffer(
      uint32_t binding,
      vk::Buffer buffer,
      vk::DeviceSize size,
      vk::DeviceSize offset,
      vk::DescriptorType desc_type
    );
    void write_image(
      uint32_t binding,
      vk::ImageView image_view,
      vk::Sampler sampler,
      vk::ImageLayout layout,
      vk::DescriptorType desc_type
    );
    void clear();
    void update_set(const vk::Device &device, vk::DescriptorSet desc_set);

  private:
    std::vector<std::tuple<vk::DescriptorBufferInfo, vk::WriteDescriptorSet>>
      buffer_infos;
    std::vector<std::tuple<vk::DescriptorImageInfo, vk::WriteDescriptorSet>>
      image_infos;
};
} // namespace kovra
