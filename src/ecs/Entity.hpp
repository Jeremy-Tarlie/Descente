#pragma once

#include <cstdint>
#include <limits>

namespace descente {

using Entity = std::uint32_t;

inline constexpr Entity kInvalidEntity = std::numeric_limits<Entity>::max();

}  // namespace descente
