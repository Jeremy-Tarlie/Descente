#include "map/Map.hpp"

#include <stdexcept>

namespace descente {

Map::Map(const int width, const int height)
    : width_(width), height_(height), tiles_(static_cast<std::size_t>(width * height)) {
  if (width <= 0 || height <= 0) {
    throw std::invalid_argument("Map dimensions must be positive");
  }
}

Tile& Map::at(const int x, const int y) {
  if (!in_bounds(x, y)) {
    throw std::out_of_range("Map::at out of bounds");
  }
  return tiles_[index(x, y)];
}

const Tile& Map::at(const int x, const int y) const {
  if (!in_bounds(x, y)) {
    throw std::out_of_range("Map::at out of bounds");
  }
  return tiles_[index(x, y)];
}

Tile& Map::at(const Point& p) { return at(p.x, p.y); }

const Tile& Map::at(const Point& p) const { return at(p.x, p.y); }

bool Map::walkable(const int x, const int y) const {
  return in_bounds(x, y) && at(x, y).walkable();
}

bool Map::walkable(const Point& p) const { return walkable(p.x, p.y); }

bool Map::blocks_sight(const int x, const int y) const {
  if (!in_bounds(x, y)) {
    return true;
  }
  return at(x, y).blocks_sight();
}

void Map::fill(const TileType type) {
  for (Tile& tile : tiles_) {
    tile.type = type;
    tile.explored = false;
    tile.visible = false;
  }
}

void Map::clear_visibility() {
  for (Tile& tile : tiles_) {
    tile.visible = false;
  }
}

void Map::mark_all_explored() {
  for (Tile& tile : tiles_) {
    tile.explored = true;
  }
}

std::optional<Point> Map::find_tile(const TileType type) const {
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      if (at(x, y).type == type) {
        return Point{x, y};
      }
    }
  }
  return std::nullopt;
}

std::vector<Point> Map::neighbors4(const Point& p) const {
  static constexpr int kDx[] = {0, 0, -1, 1};
  static constexpr int kDy[] = {-1, 1, 0, 0};
  std::vector<Point> result;
  result.reserve(4);
  for (int i = 0; i < 4; ++i) {
    const Point n{p.x + kDx[i], p.y + kDy[i]};
    if (in_bounds(n)) {
      result.push_back(n);
    }
  }
  return result;
}

}  // namespace descente
