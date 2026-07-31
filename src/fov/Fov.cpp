#include "fov/Fov.hpp"

#include <cmath>

namespace descente {

void Fov::compute(Map& map, const Point& origin, const int radius) {
  map.clear_visibility();
  if (!map.in_bounds(origin)) {
    return;
  }

  map.at(origin).visible = true;
  map.at(origin).explored = true;

  // Eight octants
  static constexpr int kMult[8][4] = {
      {1, 0, 0, 1},   {0, 1, 1, 0},   {0, -1, 1, 0},  {-1, 0, 0, 1},
      {-1, 0, 0, -1}, {0, -1, -1, 0}, {0, 1, -1, 0},  {1, 0, 0, -1},
  };

  for (int oct = 0; oct < 8; ++oct) {
    cast_light(map, origin, radius, 1, 1.0F, 0.0F, kMult[oct][0], kMult[oct][1],
               kMult[oct][2], kMult[oct][3]);
  }
}

void Fov::cast_light(Map& map, const Point& origin, const int radius,
                     const int row, float start_slope, const float end_slope,
                     const int xx, const int xy, const int yx, const int yy) {
  if (start_slope < end_slope) {
    return;
  }

  float next_start = start_slope;
  for (int i = row; i <= radius; ++i) {
    bool blocked = false;
    const int dy = -i;
    for (int dx = -i; dx <= 0; ++dx) {
      const float left_slope = (static_cast<float>(dx) - 0.5F) / (static_cast<float>(dy) + 0.5F);
      const float right_slope = (static_cast<float>(dx) + 0.5F) / (static_cast<float>(dy) - 0.5F);

      if (start_slope < right_slope) {
        continue;
      }
      if (end_slope > left_slope) {
        break;
      }

      const int map_x = origin.x + dx * xx + dy * xy;
      const int map_y = origin.y + dx * yx + dy * yy;

      const int distance = static_cast<int>(std::sqrt(
          static_cast<float>(dx * dx + dy * dy)));
      if (distance <= radius && map.in_bounds(map_x, map_y)) {
        map.at(map_x, map_y).visible = true;
        map.at(map_x, map_y).explored = true;
      }

      if (blocked) {
        if (map.in_bounds(map_x, map_y) && map.blocks_sight(map_x, map_y)) {
          next_start = right_slope;
          continue;
        }
        blocked = false;
        start_slope = next_start;
      } else if (map.in_bounds(map_x, map_y) && map.blocks_sight(map_x, map_y) &&
                 i < radius) {
        blocked = true;
        cast_light(map, origin, radius, i + 1, start_slope, left_slope, xx, xy, yx,
                   yy);
        next_start = right_slope;
      }
    }
    if (blocked) {
      break;
    }
  }
}

}  // namespace descente
