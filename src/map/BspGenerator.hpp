#pragma once

#include "map/Map.hpp"

#include <cstdint>
#include <optional>
#include <random>
#include <vector>

namespace descente {

struct Room {
  int x{0};
  int y{0};
  int w{0};
  int h{0};

  [[nodiscard]] Point center() const noexcept {
    return Point{x + w / 2, y + h / 2};
  }

  [[nodiscard]] bool contains(const int px, const int py) const noexcept {
    return px >= x && py >= y && px < x + w && py < y + h;
  }
};

struct BspConfig {
  int map_width{80};
  int map_height{40};
  int min_leaf_size{8};
  int max_leaf_size{20};
  int min_room_size{4};
  int max_depth{6};
};

struct DungeonResult {
  Map map;
  std::vector<Room> rooms;
  Point player_start{};
  Point stairs_down{};
};

class BspGenerator {
 public:
  explicit BspGenerator(std::uint32_t seed = 0);

  [[nodiscard]] DungeonResult generate(const BspConfig& config);

 private:
  struct Leaf {
    int x{0};
    int y{0};
    int w{0};
    int h{0};
    int left{-1};
    int right{-1};
    std::optional<Room> room;
  };

  bool split_leaf(const int index, const BspConfig& config, const int depth);
  void create_rooms(const BspConfig& config);
  void carve_room(Map& map, const Room& room) const;
  void carve_corridor(Map& map, Point a, Point b);
  void connect_leaves(Map& map, const int index);
  [[nodiscard]] std::optional<Room> leaf_room(const int index) const;

  std::mt19937 rng_;
  std::vector<Leaf> leaves_;
};

}  // namespace descente
