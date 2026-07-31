#pragma once

#include "game/Game.hpp"

namespace descente {

void run_ai_turn(GameState& state, Entity monster, std::mt19937& rng);
int resolve_attack(GameState& state, Entity attacker, Entity defender);
bool use_inventory_item(GameState& state, std::size_t index);

}  // namespace descente
