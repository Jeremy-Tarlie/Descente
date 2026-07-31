#include <catch2/catch_test_macros.hpp>

#include "map/Map.hpp"
#include "pathfinding/AStar.hpp"

using descente::AStar;
using descente::Map;
using descente::PathResult;
using descente::Point;
using descente::TileType;

namespace {

Map make_open_room(const int w, const int h) {
  Map map(w, h);
  map.fill(TileType::Wall);
  for (int y = 1; y < h - 1; ++y) {
    for (int x = 1; x < w - 1; ++x) {
      map.at(x, y).type = TileType::Floor;
    }
  }
  return map;
}

}  // namespace

TEST_CASE("A* finds a trivial path", "[astar]") {
  Map map = make_open_room(10, 10);
  const PathResult result = AStar::find_path(map, Point{1, 1}, Point{8, 8});
  REQUIRE(result.found);
  REQUIRE_FALSE(result.path.empty());
  REQUIRE(result.path.back() == Point{8, 8});
  REQUIRE(static_cast<int>(result.path.size()) == AStar::manhattan(Point{1, 1}, Point{8, 8}));
}

TEST_CASE("A* returns empty when start equals goal", "[astar]") {
  Map map = make_open_room(8, 8);
  const PathResult result = AStar::find_path(map, Point{3, 3}, Point{3, 3});
  REQUIRE(result.found);
  REQUIRE(result.path.empty());
}

TEST_CASE("A* fails when goal is a wall", "[astar]") {
  Map map = make_open_room(8, 8);
  const PathResult result = AStar::find_path(map, Point{2, 2}, Point{0, 0});
  REQUIRE_FALSE(result.found);
}

TEST_CASE("A* navigates around an obstacle", "[astar]") {
  Map map = make_open_room(12, 7);
  // Vertical wall with a gap
  for (int y = 1; y <= 4; ++y) {
    map.at(5, y).type = TileType::Wall;
  }
  // gap at (5,5)

  const PathResult result = AStar::find_path(map, Point{2, 2}, Point{9, 2});
  REQUIRE(result.found);
  REQUIRE(result.path.back() == Point{9, 2});

  for (const Point& p : result.path) {
    REQUIRE(map.walkable(p));
  }
}

TEST_CASE("A* respects custom blockers", "[astar]") {
  Map map = make_open_room(10, 10);
  const auto blocked = [](const Point& p) { return p.x == 4 && p.y == 3; };

  // Path must exist around the blocked cell
  const PathResult result =
      AStar::find_path(map, Point{1, 3}, Point{8, 3}, blocked);
  REQUIRE(result.found);
  for (const Point& p : result.path) {
    const bool is_blocked_cell = (p.x == 4 && p.y == 3);
    REQUIRE_FALSE(is_blocked_cell);
  }
}

TEST_CASE("Manhattan distance is symmetric", "[astar]") {
  REQUIRE(AStar::manhattan(Point{0, 0}, Point{3, 4}) == 7);
  REQUIRE(AStar::manhattan(Point{3, 4}, Point{0, 0}) == 7);
}
