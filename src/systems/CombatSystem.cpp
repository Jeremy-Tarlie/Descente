#include "systems/Systems.hpp"

#include <algorithm>
#include <sstream>

namespace descente {

int resolve_attack(GameState& state, const Entity attacker, const Entity defender) {
  Stats* atk = state.stats.get(attacker);
  Stats* def = state.stats.get(defender);
  if (atk == nullptr || def == nullptr) {
    return 0;
  }

  const int raw = std::max(1, atk->attack - def->defense);
  def->hp -= raw;

  const Name* an = state.names.get(attacker);
  const Name* dn = state.names.get(defender);
  const std::string a_name = an != nullptr ? an->value : "Quelqu'un";
  const std::string d_name = dn != nullptr ? dn->value : "quelqu'un";

  std::ostringstream oss;
  oss << a_name << " frappe " << d_name << " pour " << raw << " degats"
      << " (" << std::max(0, def->hp) << "/" << def->max_hp << " PV).";
  state.log(oss.str());

  if (def->hp <= 0) {
    def->hp = 0;
    state.log(d_name + " meurt.");

    if (state.monsters.has(defender) && state.players.has(attacker)) {
      const MonsterTag* tag = state.monsters.get(defender);
      if (tag != nullptr) {
        atk->xp += tag->xp_reward;
        std::ostringstream xp_msg;
        xp_msg << "Vous gagnez " << tag->xp_reward << " XP.";
        state.log(xp_msg.str());
      }
    }
  }
  return raw;
}

}  // namespace descente
