#pragma once

#include "map/Map.hpp"

namespace descente {

/// Recursive shadowcasting field of view (RogueBasin algorithm).
class Fov {
 public:
  static void compute(Map& map, const Point& origin, const int radius);

 private:
  static void cast_light(Map& map, const Point& origin, const int radius,
                         const int row, float start_slope, float end_slope,
                         const int xx, const int xy, const int yx, const int yy);
};

}  // namespace descente
