#pragma once

#include "buffer.hpp"

namespace kovra {
class Mesh {
  public:
    Mesh(
        const std::vector<Vertex> &vertices,
        const std::vector<uint32_t> &indices);

  private:
    std::unique_ptr<GpuBuffer> vertex_buffer;
    std::unique_ptr<GpuBuffer> index_buffer;
};
} // namespace kovra
