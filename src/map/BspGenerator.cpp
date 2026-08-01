#include "map/BspGenerator.hpp"

#include <algorithm>
#include <chrono>

namespace descente {

BspGenerator::BspGenerator(const std::uint32_t seed) {
  if (seed == 0) {
    const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    rng_.seed(static_cast<std::uint32_t>(now));
  } else {
    rng_.seed(seed);
  }
}

DungeonResult BspGenerator::generate(const BspConfig& config) {
  leaves_.clear();
  leaves_.push_back(Leaf{0, 0, config.map_width, config.map_height, -1, -1, std::nullopt});
  split_leaf(0, config, 0);

  create_rooms(config);

  Map map(config.map_width, config.map_height);
  map.fill(TileType::Wall);

  std::vector<Room> rooms;
  for (const Leaf& leaf : leaves_) {
    if (leaf.room.has_value()) {
      carve_room(map, *leaf.room);
      rooms.push_back(*leaf.room);
    }
  }

  connect_leaves(map, 0);

  if (rooms.empty()) {
    // Fallback single room
    Room fallback{2, 2, config.map_width - 4, config.map_height - 4};
    carve_room(map, fallback);
    rooms.push_back(fallback);
  }

  const Point start = rooms.front().center();
  const Point stairs = rooms.back().center();
  map.at(stairs).type = TileType::StairsDown;
  if (start != stairs) {
    map.at(start).type = TileType::Floor;
  }

  return DungeonResult{std::move(map), std::move(rooms), start, stairs};
}

bool BspGenerator::split_leaf(const int index, const BspConfig& config, const int depth) {
  // Copy fields before any push_back — a Leaf& would dangle on reallocation.
  const Leaf leaf = leaves_[static_cast<std::size_t>(index)];
  if (leaf.left >= 0 || leaf.right >= 0) {
    return false;
  }
  if (depth >= config.max_depth) {
    return false;
  }
  if (leaf.w < config.min_leaf_size * 2 && leaf.h < config.min_leaf_size * 2) {
    return false;
  }

  bool split_horizontal = false;
  if (leaf.h >= config.min_leaf_size * 2 && leaf.w >= config.min_leaf_size * 2) {
    std::uniform_int_distribution<int> coin(0, 1);
    split_horizontal = coin(rng_) == 0;
  } else if (leaf.h >= config.min_leaf_size * 2) {
    split_horizontal = true;
  } else if (leaf.w >= config.min_leaf_size * 2) {
    split_horizontal = false;
  } else {
    return false;
  }

  // Prefer splitting oversized leaves
  if (leaf.w > config.max_leaf_size && leaf.w > leaf.h) {
    split_horizontal = false;
  } else if (leaf.h > config.max_leaf_size && leaf.h >= leaf.w) {
    split_horizontal = true;
  }

  int left_idx = -1;
  int right_idx = -1;

  if (split_horizontal) {
    const int min_cut = config.min_leaf_size;
    const int max_cut = leaf.h - config.min_leaf_size;
    if (max_cut <= min_cut) {
      return false;
    }
    std::uniform_int_distribution<int> dist(min_cut, max_cut);
    const int cut = dist(rng_);
    left_idx = static_cast<int>(leaves_.size());
    leaves_.push_back(Leaf{leaf.x, leaf.y, leaf.w, cut, -1, -1, std::nullopt});
    right_idx = static_cast<int>(leaves_.size());
    leaves_.push_back(
        Leaf{leaf.x, leaf.y + cut, leaf.w, leaf.h - cut, -1, -1, std::nullopt});
  } else {
    const int min_cut = config.min_leaf_size;
    const int max_cut = leaf.w - config.min_leaf_size;
    if (max_cut <= min_cut) {
      return false;
    }
    std::uniform_int_distribution<int> dist(min_cut, max_cut);
    const int cut = dist(rng_);
    left_idx = static_cast<int>(leaves_.size());
    leaves_.push_back(Leaf{leaf.x, leaf.y, cut, leaf.h, -1, -1, std::nullopt});
    right_idx = static_cast<int>(leaves_.size());
    leaves_.push_back(
        Leaf{leaf.x + cut, leaf.y, leaf.w - cut, leaf.h, -1, -1, std::nullopt});
  }

  leaves_[static_cast<std::size_t>(index)].left = left_idx;
  leaves_[static_cast<std::size_t>(index)].right = right_idx;
  split_leaf(left_idx, config, depth + 1);
  split_leaf(right_idx, config, depth + 1);
  return true;
}

void BspGenerator::create_rooms(const BspConfig& config) {
  for (Leaf& leaf : leaves_) {
    if (leaf.left >= 0 || leaf.right >= 0) {
      continue;
    }
    const int max_w = std::max(config.min_room_size, leaf.w - 2);
    const int max_h = std::max(config.min_room_size, leaf.h - 2);
    if (max_w < config.min_room_size || max_h < config.min_room_size) {
      continue;
    }
    std::uniform_int_distribution<int> w_dist(config.min_room_size, max_w);
    std::uniform_int_distribution<int> h_dist(config.min_room_size, max_h);
    const int rw = w_dist(rng_);
    const int rh = h_dist(rng_);
    std::uniform_int_distribution<int> x_dist(0, std::max(0, leaf.w - rw - 1));
    std::uniform_int_distribution<int> y_dist(0, std::max(0, leaf.h - rh - 1));
    const int rx = leaf.x + 1 + x_dist(rng_);
    const int ry = leaf.y + 1 + y_dist(rng_);
    // Keep rooms inside leaf with 1-tile margin when possible
    const int clamped_w = std::min(rw, leaf.w - (rx - leaf.x) - 1);
    const int clamped_h = std::min(rh, leaf.h - (ry - leaf.y) - 1);
    if (clamped_w >= config.min_room_size && clamped_h >= config.min_room_size) {
      leaf.room = Room{rx, ry, clamped_w, clamped_h};
    }
  }
}

void BspGenerator::carve_room(Map& map, const Room& room) const {
  for (int y = room.y; y < room.y + room.h; ++y) {
    for (int x = room.x; x < room.x + room.w; ++x) {
      if (map.in_bounds(x, y)) {
        map.at(x, y).type = TileType::Floor;
      }
    }
  }
}

void BspGenerator::carve_corridor(Map& map, Point a, Point b) {
  std::uniform_int_distribution<int> coin(0, 1);
  if (coin(rng_) == 0) {
    // Horizontal then vertical
    const int x0 = std::min(a.x, b.x);
    const int x1 = std::max(a.x, b.x);
    for (int x = x0; x <= x1; ++x) {
      if (map.in_bounds(x, a.y)) {
        map.at(x, a.y).type = TileType::Floor;
      }
    }
    const int y0 = std::min(a.y, b.y);
    const int y1 = std::max(a.y, b.y);
    for (int y = y0; y <= y1; ++y) {
      if (map.in_bounds(b.x, y)) {
        map.at(b.x, y).type = TileType::Floor;
      }
    }
  } else {
    const int y0 = std::min(a.y, b.y);
    const int y1 = std::max(a.y, b.y);
    for (int y = y0; y <= y1; ++y) {
      if (map.in_bounds(a.x, y)) {
        map.at(a.x, y).type = TileType::Floor;
      }
    }
    const int x0 = std::min(a.x, b.x);
    const int x1 = std::max(a.x, b.x);
    for (int x = x0; x <= x1; ++x) {
      if (map.in_bounds(x, b.y)) {
        map.at(x, b.y).type = TileType::Floor;
      }
    }
  }
}

std::optional<Room> BspGenerator::leaf_room(const int index) const {
  const Leaf& leaf = leaves_[static_cast<std::size_t>(index)];
  if (leaf.room.has_value()) {
    return leaf.room;
  }
  if (leaf.left < 0) {
    return std::nullopt;
  }
  const auto left = leaf_room(leaf.left);
  if (left.has_value()) {
    return left;
  }
  return leaf_room(leaf.right);
}

void BspGenerator::connect_leaves(Map& map, const int index) {
  const Leaf& leaf = leaves_[static_cast<std::size_t>(index)];
  if (leaf.left < 0 || leaf.right < 0) {
    return;
  }
  connect_leaves(map, leaf.left);
  connect_leaves(map, leaf.right);
  const auto a = leaf_room(leaf.left);
  const auto b = leaf_room(leaf.right);
  if (a.has_value() && b.has_value()) {
    carve_corridor(map, a->center(), b->center());
  }
}

}  // namespace descente
