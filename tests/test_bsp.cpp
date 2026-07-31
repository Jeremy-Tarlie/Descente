#include <catch2/catch_test_macros.hpp>

#include "map/BspGenerator.hpp"

using descente::BspConfig;
using descente::BspGenerator;
using descente::TileType;

TEST_CASE("BSP generates a walkable dungeon with stairs", "[bsp]") {
  BspGenerator gen(42);
  BspConfig config;
  config.map_width = 60;
  config.map_height = 30;
  config.min_leaf_size = 8;
  config.max_leaf_size = 18;
  config.min_room_size = 4;
  config.max_depth = 5;

  const auto dungeon = gen.generate(config);

  REQUIRE(dungeon.map.width() == 60);
  REQUIRE(dungeon.map.height() == 30);
  REQUIRE_FALSE(dungeon.rooms.empty());
  REQUIRE(dungeon.map.walkable(dungeon.player_start));
  REQUIRE(dungeon.map.at(dungeon.stairs_down).type == TileType::StairsDown);

  int floor_tiles = 0;
  for (int y = 0; y < dungeon.map.height(); ++y) {
    for (int x = 0; x < dungeon.map.width(); ++x) {
      if (dungeon.map.walkable(x, y)) {
        ++floor_tiles;
      }
    }
  }
  REQUIRE(floor_tiles > 50);
}

TEST_CASE("BSP is deterministic for a fixed seed", "[bsp]") {
  BspConfig config;
  config.map_width = 40;
  config.map_height = 25;

  const auto a = BspGenerator(12345).generate(config);
  const auto b = BspGenerator(12345).generate(config);

  REQUIRE(a.rooms.size() == b.rooms.size());
  REQUIRE(a.player_start == b.player_start);
  REQUIRE(a.stairs_down == b.stairs_down);

  for (int y = 0; y < a.map.height(); ++y) {
    for (int x = 0; x < a.map.width(); ++x) {
      REQUIRE(a.map.at(x, y).type == b.map.at(x, y).type);
    }
  }
}

TEST_CASE("BSP rooms stay inside map bounds", "[bsp]") {
  BspGenerator gen(7);
  BspConfig config;
  config.map_width = 50;
  config.map_height = 40;

  const auto dungeon = gen.generate(config);
  for (const auto& room : dungeon.rooms) {
    REQUIRE(room.x >= 0);
    REQUIRE(room.y >= 0);
    REQUIRE(room.x + room.w <= config.map_width);
    REQUIRE(room.y + room.h <= config.map_height);
    REQUIRE(room.w >= config.min_room_size);
    REQUIRE(room.h >= config.min_room_size);
  }
}
