#pragma once

#include <cstdint>

namespace descente {

enum class InputAction : std::uint8_t {
  None,
  Quit,
  NewGame,
  MoveN,
  MoveS,
  MoveE,
  MoveW,
  MoveNE,
  MoveNW,
  MoveSE,
  MoveSW,
  Wait,
  Pickup,
  ToggleInventory,
  CloseInventory,
  StairsDown,
  Save,
  Load,
  UseSlot0,
  UseSlot1,
  UseSlot2,
  UseSlot3,
  UseSlot4,
  UseSlot5,
  UseSlot6,
  UseSlot7,
  UseSlot8,
  UseSlot9,
};

}  // namespace descente
