#include "render_object.hpp"
#include "asset_loader.hpp"

namespace kovra {

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
