#pragma once

#include "game/Game.hpp"

#include <string>

namespace descente {

bool save_game(const GameState& state, const std::string& path);
bool load_game(GameState& state, const std::string& path, std::mt19937& rng);

}  // namespace descente
