#pragma once

#include "draw_context.hpp"
#include "material.hpp"
#include "mesh.hpp"

#include "glm/mat4x4.hpp"
#include "spdlog/spdlog.h"
#include <memory>
#include <vulkan/vulkan.hpp>

namespace kovra {
// Forward declarations
struct MeshAsset;

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
  public:
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

    [[nodiscard]] bool has_parent() const noexcept { return !parent.expired(); }

    void add_child(
      std::shared_ptr<SceneNode> child,
      bool refresh_world_transforms = false
    )
    {
        children.push_back(child);
        child->parent = shared_from_this();
        if (refresh_world_transforms) {
            child->refresh_world_transform(world_transform);
        }
    }

    void set_local_transform(
      const glm::mat4 &transform,
      bool refresh_world_transforms = false
    )
    {
        local_transform = transform;

        if (!refresh_world_transforms) {
            return;
        }
        if (has_parent()) {
            auto parent_owned = parent.lock();
            refresh_world_transform(parent_owned->world_transform);
        } else {
            refresh_world_transform(glm::identity<glm::mat4>());
        }
    }

    // Refresh the world transform of this node and all its children
    void refresh_world_transform(const glm::mat4 &parent_world_transform)
    {
        world_transform = parent_world_transform * local_transform;
        for (auto &child : children) {
            child->refresh_world_transform(world_transform);
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
};

class MeshNode : public SceneNode
{
  public:
    MeshNode(std::shared_ptr<MeshAsset> mesh_asset)
      : mesh_asset{ mesh_asset }
    {
    }
    MeshNode() = delete;
    MeshNode(const MeshNode &) = delete;
    MeshNode &operator=(const MeshNode &) = delete;
    MeshNode(MeshNode &&) = delete;
    MeshNode &operator=(MeshNode &&) = delete;

    [[nodiscard]] const MeshAsset &get_mesh_asset() const
    {
        return *mesh_asset;
    }
    [[nodiscard]] MeshAsset &get_mesh_asset_mut() const { return *mesh_asset; }

    virtual void queue_draw(const glm::mat4 &root_transform, DrawContext &ctx)
      const override;

  private:
    std::shared_ptr<MeshAsset> mesh_asset;
};
} // namespace kovra
