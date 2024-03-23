#include "descriptor.hpp"
#include <algorithm>

namespace kovra {
std::vector<DescriptorPoolSizeRatio> create_pool_size_ratios();
vk::UniqueDescriptorPool create_pool(
    const vk::Device &device, uint32_t set_count,
    const std::vector<DescriptorPoolSizeRatio> &ratios);

DescriptorSetLayoutBuilder &DescriptorSetLayoutBuilder::add_binding(
    uint32_t binding, vk::DescriptorType descriptor_type,
    vk::ShaderStageFlags stage_flags) {
    bindings.emplace_back(vk::DescriptorSetLayoutBinding{}
                              .setBinding(binding)
                              .setDescriptorType(descriptor_type)
                              .setDescriptorCount(1)
                              .setStageFlags(stage_flags));
    return *this;
}

vk::DescriptorSetLayout
DescriptorSetLayoutBuilder::build(const vk::Device &device) const {
    return device.createDescriptorSetLayout(
        vk::DescriptorSetLayoutCreateInfo{}.setBindings(bindings));
}

DescriptorAllocator::DescriptorAllocator(
    const vk::Device &device, uint32_t max_sets)
    : pool_ratios{create_pool_size_ratios()} {
    ready_pools.push_back(create_pool(device, max_sets, pool_ratios));
    sets_per_pool = static_cast<uint32_t>(max_sets * 1.5);
}

DescriptorAllocator::~DescriptorAllocator() {
    // Destroy all pools
    destroy_pools();
}

vk::DescriptorSet DescriptorAllocator::allocate(
    vk::DescriptorSetLayout layout, const vk::Device &device) {
    auto pool_to_use = get_next_ready_pool(device);

    // Create descriptor set allocation info
    VkDescriptorPool vk_pool_to_use =
        static_cast<VkDescriptorPool>(pool_to_use.get());
    VkDescriptorSetLayout vk_layout =
        static_cast<VkDescriptorSetLayout>(layout);
    VkDevice vk_device = static_cast<VkDevice>(device);
    VkDescriptorSetAllocateInfo vk_alloc_info{};
    vk_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    vk_alloc_info.pNext = nullptr;
    vk_alloc_info.descriptorPool = vk_pool_to_use;
    vk_alloc_info.descriptorSetCount = 1;
    vk_alloc_info.pSetLayouts = &vk_layout;

    // Try to allocate a descriptor set
    VkDescriptorSet vk_desc_set;
    VkResult result =
        vkAllocateDescriptorSets(vk_device, &vk_alloc_info, &vk_desc_set);

    // Check if allocation failed because the pool ran out of memory
    if (result == VK_ERROR_OUT_OF_POOL_MEMORY ||
        result == VK_ERROR_FRAGMENTED_POOL) {
        full_pools.push_back(std::move(pool_to_use));
        pool_to_use = get_next_ready_pool(device);

        // Allocate the descriptor set again
        result =
            vkAllocateDescriptorSets(vk_device, &vk_alloc_info, &vk_desc_set);
        // If allocation still fails, throw an exception because this descriptor
        // allocator is broken
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate descriptor set");
        }
    }

    ready_pools.push_back(std::move(pool_to_use));
    return vk_desc_set;
}

void DescriptorAllocator::clear_pools(const vk::Device &device) {
    // Insert all full pools into the ready pools list
    ready_pools.insert(
        ready_pools.end(), std::make_move_iterator(full_pools.begin()),
        std::make_move_iterator(full_pools.end()));
    // Reset all pools
    for (auto &pool : ready_pools) {
        device.resetDescriptorPool(pool.get());
    }
    // Clear full pools list
    full_pools.clear();
}

void DescriptorAllocator::destroy_pools() {
    for (auto &pool : ready_pools) {
        pool.reset();
    }
    ready_pools.clear();
    for (auto &pool : full_pools) {
        pool.reset();
    }
    full_pools.clear();
}

vk::UniqueDescriptorPool
DescriptorAllocator::get_next_ready_pool(const vk::Device &device) {
    // If ran out of pools, create a new one
    if (ready_pools.empty()) {
        ready_pools.push_back(create_pool(device, sets_per_pool, pool_ratios));
        // Increase number of sets per pool
        uint32_t new_sets_per_pool = static_cast<uint32_t>(sets_per_pool * 1.5);
        // Limit max sets per pool to 4092
        sets_per_pool =
            std::min(new_sets_per_pool, static_cast<uint32_t>(4092));
    }
    // Return the last pool in the ready list
    auto pool = std::move(ready_pools.back());
    ready_pools.pop_back();
    return pool;
}

void DescriptorWriter::write_buffer(
    uint32_t binding, vk::Buffer buffer, vk::DeviceSize size,
    vk::DeviceSize offset, vk::DescriptorType desc_type) {
    auto buffer_info =
        vk::DescriptorBufferInfo{}.setBuffer(buffer).setOffset(offset).setRange(
            size);
    auto buffer_write = vk::WriteDescriptorSet{}
                            .setDstBinding(binding)
                            .setDescriptorCount(1)
                            .setDescriptorType(desc_type);
    buffer_infos.emplace_back(std::make_tuple(buffer_info, buffer_write));
}

void DescriptorWriter::write_image(
    uint32_t binding, vk::ImageView image_view, vk::Sampler sampler,
    vk::ImageLayout layout, vk::DescriptorType desc_type) {
    auto image_info = vk::DescriptorImageInfo{}
                          .setSampler(sampler)
                          .setImageView(image_view)
                          .setImageLayout(layout);
    auto image_write = vk::WriteDescriptorSet{}
                           .setDstBinding(binding)
                           .setDescriptorCount(1)
                           .setDescriptorType(desc_type);
    image_infos.emplace_back(std::make_tuple(image_info, image_write));
}

void DescriptorWriter::clear() {
    buffer_infos.clear();
    image_infos.clear();
}

void DescriptorWriter::update_set(
    const vk::Device &device, vk::DescriptorSet desc_set) {
    std::vector<vk::WriteDescriptorSet> writes;
    for (auto &[info, write] : buffer_infos) {
        write.setDstSet(desc_set);
        write.setPBufferInfo(&info);
        writes.push_back(write);
    }
    for (auto &[info, write] : image_infos) {
        write.setDstSet(desc_set);
        write.setPImageInfo(&info);
        writes.push_back(write);
    }
    device.updateDescriptorSets(writes, nullptr);
}

std::vector<DescriptorPoolSizeRatio> create_pool_size_ratios() {
    return {
        DescriptorPoolSizeRatio{vk::DescriptorType::eUniformBuffer, 3.0f},
        DescriptorPoolSizeRatio{vk::DescriptorType::eStorageBuffer, 3.0f},
        DescriptorPoolSizeRatio{
            vk::DescriptorType::eCombinedImageSampler, 4.0f},
        DescriptorPoolSizeRatio{vk::DescriptorType::eStorageImage, 3.0f},
    };
}

vk::UniqueDescriptorPool create_pool(
    const vk::Device &device, uint32_t set_count,
    const std::vector<DescriptorPoolSizeRatio> &ratios) {
    std::vector<vk::DescriptorPoolSize> pool_sizes;
    pool_sizes.reserve(ratios.size());
    for (const auto &ratio : ratios) {
        pool_sizes.push_back(vk::DescriptorPoolSize{}
                                 .setType(ratio.desc_type)
                                 .setDescriptorCount(static_cast<uint32_t>(
                                     set_count * ratio.ratio)));
    }
    return device.createDescriptorPoolUnique(
        vk::DescriptorPoolCreateInfo{}.setMaxSets(set_count).setPoolSizes(
            pool_sizes));
}
} // namespace kovra
