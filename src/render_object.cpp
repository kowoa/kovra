#include "render_object.hpp"
#include "asset_loader.hpp"

namespace kovra {

[[nodiscard]] bool
RenderObject::is_visible(const glm::mat4 &viewproj) const noexcept
{
    std::array<glm::vec3, 8> corners{
        glm::vec3{ 1, 1, 1 },   glm::vec3{ 1, 1, -1 },   glm::vec3{ 1, -1, 1 },
        glm::vec3{ 1, -1, -1 }, glm::vec3{ -1, 1, 1 },   glm::vec3{ -1, 1, -1 },
        glm::vec3{ -1, -1, 1 }, glm::vec3{ -1, -1, -1 },
    };

    glm::mat4 clip_space_transform = viewproj * transform;

    // Initialize the bounding box
    glm::vec3 min{ 1.5f, 1.5f, 1.5f };
    glm::vec3 max{ -1.5f, -1.5f, -1.5f };

    for (size_t i = 0; i < 8; i++) {
        // Project each corner into clip space
        glm::vec4 v =
          clip_space_transform *
          glm::vec4(bounds.origin + (corners[i] * bounds.extents), 1.0f);

        // Perspective divide/correction
        v /= v.w;

        // Update the bounding box
        min = glm::min(min, glm::vec3(v));
        max = glm::max(max, glm::vec3(v));
    }

    // Check if the clip space box is within the view
    if (max.x < -1 || min.x > 1 || max.y < -1 || min.y > 1 || max.z < 0 || min.z > 1) {
        return false;
    }
    return true;
}

void
MeshNode::queue_draw(const glm::mat4 &root_transform, DrawContext &ctx) const
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
            spdlog::warn("MeshNode::draw: {} has no Mesh", mesh_asset->name);
            continue;
        }

        auto render_object =
          RenderObject{ .index_count = surface.count,
                        .first_index = surface.start_index,
                        .index_buffer =
                          mesh_asset->mesh->get_index_buffer().get(),
                        .material_instance = surface.material_instance,
                        .bounds = surface.bounds,
                        .transform = node_transform,
                        .vertex_buffer_address =
                          mesh_asset->mesh->get_vertex_buffer_address() };

        if (surface.material_instance->pass == MaterialPass::Opaque) {
            ctx.opaque_objects.emplace_back(std::move(render_object));
        } else if (surface.material_instance->pass == MaterialPass::Transparent) {
            ctx.transparent_objects.emplace_back(std::move(render_object));
        } else {
            spdlog::error(
              "MeshNode::draw: {} has unknown MaterialPass", mesh_asset->name
            );
        }
    }

    SceneNode::queue_draw(root_transform, ctx);
}
}
