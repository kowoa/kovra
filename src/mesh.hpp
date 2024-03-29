#pragma once

#include "buffer.hpp"
#include "vertex.hpp"

namespace kovra {
// Forward declarations
class Device;

class Mesh
{
  public:
    Mesh(
      const std::span<Vertex> &vertices,
      const std::span<uint32_t> &indices,
      const Device &device
    );
    ~Mesh();
    Mesh() = delete;
    Mesh(const Mesh &) = delete;
    Mesh &operator=(const Mesh &) = delete;
    Mesh(Mesh &&) = delete;
    Mesh &operator=(Mesh &&) = delete;

  private:
    static uint32_t MESH_ID_COUNTER;
    uint32_t id;

    std::unique_ptr<GpuBuffer> vertex_buffer;
    std::unique_ptr<GpuBuffer> index_buffer;
    vk::DeviceAddress vertex_buffer_address;
};
} // namespace kovra
