#pragma once

#include "map/Map.hpp"

#include <cmath>
#include <cstdlib>
#include <functional>
#include <vector>

namespace descente {

struct PathResult {
  std::vector<Point> path;  // excludes start, includes goal if reached
  bool found{false};
};

/// A* pathfinding on a walkable grid (4-directional).
class AStar {
 public:
  using BlockedFn = std::function<bool(const Point&)>;

  [[nodiscard]] static PathResult find_path(const Map& map, const Point& start,
                                            const Point& goal,
                                            const BlockedFn& is_blocked = {});

  [[nodiscard]] static int manhattan(const Point& a, const Point& b) noexcept {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
  }
};

}  // namespace descente
