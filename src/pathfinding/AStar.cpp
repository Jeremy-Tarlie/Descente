#include "pathfinding/AStar.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>

namespace descente {

namespace {

struct Node {
  Point pos{};
  int g{0};
  int f{0};
};

struct NodeGreater {
  bool operator()(const Node& a, const Node& b) const noexcept {
    return a.f > b.f;
  }
};

}  // namespace

PathResult AStar::find_path(const Map& map, const Point& start, const Point& goal,
                            const BlockedFn& is_blocked) {
  PathResult result;
  if (!map.in_bounds(start) || !map.in_bounds(goal)) {
    return result;
  }
  if (start == goal) {
    result.found = true;
    return result;
  }
  if (!map.walkable(goal)) {
    return result;
  }

  const int w = map.width();
  const int h = map.height();
  const auto idx = [w](const Point& p) -> std::size_t {
    return static_cast<std::size_t>(p.y * w + p.x);
  };

  const std::size_t cell_count = static_cast<std::size_t>(w * h);
  std::vector<int> g_score(cell_count, std::numeric_limits<int>::max());
  std::vector<Point> came_from(cell_count, Point{-1, -1});
  std::vector<bool> closed(cell_count, false);

  std::priority_queue<Node, std::vector<Node>, NodeGreater> open;
  g_score[idx(start)] = 0;
  open.push(Node{start, 0, manhattan(start, goal)});

  static constexpr int kDx[] = {0, 0, -1, 1};
  static constexpr int kDy[] = {-1, 1, 0, 0};

  while (!open.empty()) {
    const Node current = open.top();
    open.pop();
    const std::size_t current_i = idx(current.pos);

    if (closed[current_i]) {
      continue;
    }
    closed[current_i] = true;

    if (current.pos == goal) {
      result.found = true;
      Point p = goal;
      while (p != start) {
        result.path.push_back(p);
        p = came_from[idx(p)];
        if (p.x < 0) {
          result.found = false;
          result.path.clear();
          return result;
        }
      }
      std::reverse(result.path.begin(), result.path.end());
      return result;
    }

    for (int d = 0; d < 4; ++d) {
      const Point next{current.pos.x + kDx[d], current.pos.y + kDy[d]};
      if (!map.in_bounds(next) || !map.walkable(next)) {
        continue;
      }
      if (is_blocked && is_blocked(next) && next != goal) {
        continue;
      }
      const std::size_t next_i = idx(next);
      if (closed[next_i]) {
        continue;
      }
      const int tentative_g = g_score[current_i] + 1;
      if (tentative_g >= g_score[next_i]) {
        continue;
      }
      came_from[next_i] = current.pos;
      g_score[next_i] = tentative_g;
      open.push(Node{next, tentative_g, tentative_g + manhattan(next, goal)});
    }
  }

  return result;
}

}  // namespace descente
