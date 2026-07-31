#include "systems/Systems.hpp"

#include <algorithm>
#include <sstream>
#include <vector>

namespace descente {

bool use_inventory_item(GameState& state, const std::size_t index) {
  Inventory* inv = state.inventories.get(state.player);
  Stats* stats = state.stats.get(state.player);
  Position* pos = state.positions.get(state.player);
  if (inv == nullptr || stats == nullptr || pos == nullptr) {
    return false;
  }
  if (index >= inv->items.size()) {
    return false;
  }

  const Item item = inv->items[index];
  inv->items.erase(inv->items.begin() + static_cast<std::ptrdiff_t>(index));

  switch (item.kind) {
    case ItemKind::HealthPotion: {
      const int heal = std::min(12, stats->max_hp - stats->hp);
      stats->hp += heal;
      std::ostringstream oss;
      oss << "Vous buvez " << item.name << " (+" << heal << " PV).";
      state.log(oss.str());
      break;
    }
    case ItemKind::StrengthPotion: {
      stats->attack += 1;
      state.log("Vous buvez " + item.name + " (+1 attaque).");
      break;
    }
    case ItemKind::EscapeScroll: {
      std::vector<Point> candidates;
      for (int y = 0; y < state.map.height(); ++y) {
        for (int x = 0; x < state.map.width(); ++x) {
          const Point p{x, y};
          if (state.map.walkable(p) && state.blocker_at(p) == kInvalidEntity) {
            candidates.push_back(p);
          }
        }
      }
      if (!candidates.empty()) {
        const Point dest = candidates[candidates.size() / 2];
        pos->set(dest);
        state.log("Le parchemin vous teleporte ailleurs.");
      } else {
        state.log("Le parchemin fume... sans effet.");
      }
      break;
    }
    case ItemKind::AbyssalShard: {
      state.log("L'Eclat Abyssal eclate de lumiere. Victoire !");
      state.phase = GamePhase::Won;
      break;
    }
  }
  return true;
}

}  // namespace descente
