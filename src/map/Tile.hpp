#pragma once

#include <cstdint>

namespace descente {

enum class TileType : std::uint8_t {
  Wall,
  Floor,
  StairsDown,
  StairsUp,
};

struct Tile {
  TileType type{TileType::Wall};
  bool explored{false};
  bool visible{false};

  [[nodiscard]] bool walkable() const noexcept {
    return type == TileType::Floor || type == TileType::StairsDown ||
           type == TileType::StairsUp;
  }

  [[nodiscard]] bool blocks_sight() const noexcept {
    return type == TileType::Wall;
  }

  [[nodiscard]] char glyph() const noexcept {
    switch (type) {
      case TileType::Wall:
        return '#';
      case TileType::Floor:
        return '.';
      case TileType::StairsDown:
        return '>';
      case TileType::StairsUp:
        return '<';
    }
    return '?';
  }
};

}  // namespace descente
