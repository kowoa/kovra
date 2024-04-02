#pragma once

#include "asset_loader.hpp"
#include "draw_context.hpp"
#include "material.hpp"
#include "mesh.hpp"

#include "glm/mat4x4.hpp"
#include "spdlog/spdlog.h"
#include <memory>
#include <vulkan/vulkan.hpp>

namespace kovra {

// WARNING: Do not store this struct as a member of a class.
// It contains references to Vulkan objects that may be destroyed.
// Instead, create this struct dynamically each frame.
struct RenderObject
{
    const uint32_t index_count;
    const uint32_t first_index;
    const vk::Buffer index_buffer;

    const std::shared_ptr<MaterialInstance> material_instance;

    const glm::mat4 transform;
    const vk::DeviceAddress vertex_buffer_address;
};

// Base class for a renderable dynamic object
class IRenderable
{
    // Modify the DrawContext to add the object to the render queue
    virtual void queue_draw(const glm::mat4 &parent_transform, DrawContext &ctx)
      const = 0;
};

class SceneNode
  : public IRenderable
  , public std::enable_shared_from_this<SceneNode>
{
  public:
    SceneNode()
      : parent{}
      , children{}
      , local_transform{ glm::identity<glm::mat4>() }
      , world_transform{ glm::identity<glm::mat4>() }
    {
    }

    SceneNode(const SceneNode &) = delete;
    SceneNode &operator=(const SceneNode &) = delete;
    SceneNode(SceneNode &&) = delete;
    SceneNode &operator=(SceneNode &&) = delete;

    void add_child(std::shared_ptr<SceneNode> child)
    {
        children.push_back(child);
        child->parent = shared_from_this();
        child->refresh_world_transform(world_transform);
    }

    void set_local_transform(const glm::mat4 &transform)
    {
        local_transform = transform;

        if (!parent.expired()) {
            auto parent_owned = parent.lock();
            refresh_world_transform(parent_owned->world_transform);
        } else {
            refresh_world_transform(glm::identity<glm::mat4>());
        }
    }

    virtual void queue_draw(const glm::mat4 &root_transform, DrawContext &ctx)
      const override
    {
        for (auto &child : children) {
            child->queue_draw(root_transform, ctx);
        }
    }

  protected:
    // Parent pointer must be a weak pointer to avoid circular dependencies
    std::weak_ptr<SceneNode> parent;
    std::vector<std::shared_ptr<SceneNode>> children;
    glm::mat4 local_transform;
    glm::mat4 world_transform;

    void refresh_world_transform(const glm::mat4 &parent_world_transform)
    {
        world_transform = parent_world_transform * local_transform;
        for (auto &child : children) {
            child->refresh_world_transform(world_transform);
        }
    }
};

class MeshNode : public SceneNode
{
  public:
    MeshNode(std::shared_ptr<MeshAsset> mesh_asset)
      : mesh_asset{ mesh_asset }
    {
    }

    [[nodiscard]] const MeshAsset &get_mesh_asset() const
    {
        return *mesh_asset;
    }
    [[nodiscard]] MeshAsset &get_mesh_asset_mut() const { return *mesh_asset; }

    virtual void queue_draw(const glm::mat4 &root_transform, DrawContext &ctx)
      const override
    {
        glm::mat4 node_transform = root_transform * world_transform;

        for (const auto &surface : mesh_asset->surfaces) {
            if (surface.material_instance == nullptr) {
                spdlog::warn(
                  "MeshNode::draw: {}'s GeometrySurface has no "
                  "MaterialInstance",
                  mesh_asset->name
                );
                continue;
            }
            if (mesh_asset->mesh == nullptr) {
                spdlog::warn(
                  "MeshNode::draw: {} has no Mesh", mesh_asset->name
                );
                continue;
            }
            ctx.opaque_objects.emplace_back(RenderObject{
              .index_count = surface.count,
              .first_index = surface.start_index,
              .index_buffer = mesh_asset->mesh->get_index_buffer().get(),
              .material_instance = surface.material_instance,
              .transform = node_transform,
              .vertex_buffer_address =
                mesh_asset->mesh->get_vertex_buffer_address() });
        }

        SceneNode::queue_draw(root_transform, ctx);
    }

  private:
    std::shared_ptr<MeshAsset> mesh_asset;
};
} // namespace kovra
