#pragma once

#include "game/Game.hpp"
#include "game/Input.hpp"

namespace descente {

/// Poll keyboard/mouse once per frame; returns at most one gameplay action.
[[nodiscard]] InputAction poll_input(const GameState& state);

}  // namespace descente
