#pragma once

#include "map/Tile.hpp"

#include <optional>
#include <utility>
#include <vector>

namespace descente {

struct Point {
  int x{0};
  int y{0};

  [[nodiscard]] bool operator==(const Point& other) const noexcept {
    return x == other.x && y == other.y;
  }

  [[nodiscard]] bool operator!=(const Point& other) const noexcept {
    return !(*this == other);
  }
};

class Map {
 public:
  Map() = default;
  Map(const int width, const int height);

  [[nodiscard]] int width() const noexcept { return width_; }
  [[nodiscard]] int height() const noexcept { return height_; }

  [[nodiscard]] bool in_bounds(const int x, const int y) const noexcept {
    return x >= 0 && y >= 0 && x < width_ && y < height_;
  }

  [[nodiscard]] bool in_bounds(const Point& p) const noexcept {
    return in_bounds(p.x, p.y);
  }

  [[nodiscard]] Tile& at(const int x, const int y);
  [[nodiscard]] const Tile& at(const int x, const int y) const;
  [[nodiscard]] Tile& at(const Point& p);
  [[nodiscard]] const Tile& at(const Point& p) const;

  [[nodiscard]] bool walkable(const int x, const int y) const;
  [[nodiscard]] bool walkable(const Point& p) const;
  [[nodiscard]] bool blocks_sight(const int x, const int y) const;

  void fill(const TileType type);
  void clear_visibility();
  void mark_all_explored();

  [[nodiscard]] std::optional<Point> find_tile(const TileType type) const;
  [[nodiscard]] std::vector<Point> neighbors4(const Point& p) const;

  [[nodiscard]] const std::vector<Tile>& tiles() const noexcept { return tiles_; }
  std::vector<Tile>& tiles() noexcept { return tiles_; }

 private:
  [[nodiscard]] std::size_t index(const int x, const int y) const noexcept {
    return static_cast<std::size_t>(y * width_ + x);
  }

  int width_{0};
  int height_{0};
  std::vector<Tile> tiles_;
};

}  // namespace descente
