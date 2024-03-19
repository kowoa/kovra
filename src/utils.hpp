#pragma once

#include <cstdint>

namespace kovra {
namespace utils {
inline uint32_t aligned_size(uint32_t size, uint32_t alignment) {
    if (alignment == 0) {
        return size;
    }
    return (size + alignment - 1) & ~(alignment - 1);
}
} // namespace utils
} // namespace kovra
