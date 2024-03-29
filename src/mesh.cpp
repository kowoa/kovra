#include "mesh.hpp"
#include "device.hpp"

namespace kovra {
void
upload(
  const std::span<GpuVertexData> &vertices,
  const std::span<uint32_t> &indices,
  const GpuBuffer &vertex_buffer,
  const GpuBuffer &index_buffer,
  const Device &device
);

uint32_t Mesh::MESH_ID_COUNTER = 0;

Mesh::Mesh(
  const std::span<Vertex> &vertices,
  const std::span<uint32_t> &indices,
  const Device &device
)
  : id{ MESH_ID_COUNTER++ }
  , vertex_buffer{ device.create_buffer(
      sizeof(GpuVertexData) * vertices.size(),
      vk::BufferUsageFlagBits::eStorageBuffer |
        vk::BufferUsageFlagBits::eTransferDst |
        vk::BufferUsageFlagBits::eShaderDeviceAddress,
      VMA_MEMORY_USAGE_GPU_ONLY,
      VMA_ALLOCATION_CREATE_MAPPED_BIT
    ) }
  , index_buffer{ device.create_buffer(
      sizeof(uint32_t) * indices.size(),
      vk::BufferUsageFlagBits::eIndexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
      VMA_MEMORY_USAGE_GPU_ONLY,
      VMA_ALLOCATION_CREATE_MAPPED_BIT
    ) }
  , vertex_buffer_address{ device.get().getBufferAddress(
      vk::BufferDeviceAddressInfo{}.setBuffer(vertex_buffer->get())
    ) }
{
    // Convert each Vertex to GpuVertexData
    std::vector<GpuVertexData> gpu_vertices;
    gpu_vertices.reserve(vertices.size());
    for (const Vertex &vertex : vertices) {
        gpu_vertices.emplace_back(vertex.as_gpu_data());
    }

    // Upload vertices and indices to GPU
    upload(gpu_vertices, indices, *vertex_buffer, *index_buffer, device);
}
Mesh::~Mesh()
{
    index_buffer.reset();
    vertex_buffer.reset();
}

void
upload(
  const std::span<GpuVertexData> &vertices,
  const std::span<uint32_t> &indices,
  const GpuBuffer &vertex_buffer,
  const GpuBuffer &index_buffer,
  const Device &device
)
{
    const size_t vertex_buffer_size = sizeof(GpuVertexData) * vertices.size();
    const size_t index_buffer_size = sizeof(uint32_t) * indices.size();

    // Create staging buffer
    auto staging_buffer = device.create_buffer(
      vertex_buffer_size + index_buffer_size,
      vk::BufferUsageFlagBits::eTransferSrc,
      VMA_MEMORY_USAGE_CPU_ONLY,
      VMA_ALLOCATION_CREATE_MAPPED_BIT
    );

    // Write to staging buffer
    staging_buffer->write(vertices.data(), vertex_buffer_size, 0);
    staging_buffer->write(
      indices.data(), index_buffer_size, vertex_buffer_size
    );

    // Copy staging buffer to vertex and index buffers
    device.immediate_submit([&](vk::CommandBuffer cmd) {
        auto vertex_copy =
          vk::BufferCopy{}.setSrcOffset(0).setDstOffset(0).setSize(
            vertex_buffer_size
          );
        cmd.copyBuffer(staging_buffer->get(), vertex_buffer.get(), vertex_copy);

        auto index_copy = vk::BufferCopy{}
                            .setSrcOffset(vertex_buffer_size)
                            .setDstOffset(0)
                            .setSize(index_buffer_size);
        cmd.copyBuffer(staging_buffer->get(), index_buffer.get(), index_copy);
    });
}
} // namespace kovra
