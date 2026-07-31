#include "game/Game.hpp"

#include "fov/Fov.hpp"
#include "game/SaveLoad.hpp"
#include "systems/Systems.hpp"

#include <chrono>
#include <sstream>
#include <vector>

namespace descente {

namespace {

constexpr int kFovRadius = 10;
constexpr int kMaxFloor = 5;
constexpr const char* kSavePath = "descente_save.dat";

}  // namespace

Game::Game() {
  bsp_config_.map_width = 80;
  bsp_config_.map_height = 40;
  bsp_config_.min_leaf_size = 8;
  bsp_config_.max_leaf_size = 22;
  bsp_config_.min_room_size = 4;
  bsp_config_.max_depth = 5;
}

void Game::new_game(const std::uint32_t seed) {
  if (seed == 0) {
    const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    state_.seed = static_cast<std::uint32_t>(now);
  } else {
    state_.seed = seed;
  }
  rng_.seed(state_.seed);
  state_.floor = 1;
  state_.turn = 0;
  state_.phase = GamePhase::Playing;
  state_.messages.clear();
  state_.log("But : descendez a l'etage 5, vainquez le Gardien,");
  state_.log("puis ramassez l'Eclat Abyssal pour gagner.");
  state_.log("I = inventaire (clic ou 1-9 pour utiliser). G = ramasser.");
  generate_floor();
  if (Inventory* inv = state_.inventories.get(state_.player)) {
    inv->try_add(make_health_potion());
  }
}

void Game::next_floor() {
  if (state_.floor >= kMaxFloor) {
    state_.log("Dernier etage : pas d'escalier. Affrontez le Gardien !");
    state_.needs_redraw = true;
    return;
  }

  ++state_.floor;
  std::ostringstream oss;
  oss << "Vous descendez a l'etage " << state_.floor << "/" << kMaxFloor << ".";
  state_.log(oss.str());
  if (state_.floor == kMaxFloor) {
    state_.log("Le Gardien Abyssal protege l'Eclat. Tuez-le !");
  }

  // Preserve player stats/inventory
  Stats saved_stats{};
  Inventory saved_inv{};
  if (const Stats* s = state_.stats.get(state_.player)) {
    saved_stats = *s;
  }
  if (const Inventory* inv = state_.inventories.get(state_.player)) {
    saved_inv = *inv;
  }

  generate_floor();

  if (Stats* s = state_.stats.get(state_.player)) {
    *s = saved_stats;
  }
  if (Inventory* inv = state_.inventories.get(state_.player)) {
    *inv = std::move(saved_inv);
  }
}

void Game::generate_floor() {
  BspGenerator gen(state_.seed + static_cast<std::uint32_t>(state_.floor) * 9973U);
  const DungeonResult dungeon = gen.generate(bsp_config_);
  state_.map = std::move(dungeon.map);
  state_.rooms = dungeon.rooms;

  Point boss_spot = dungeon.stairs_down;
  if (state_.floor == kMaxFloor) {
    // No exit stairs on the final floor — the boss waits there.
    if (state_.map.in_bounds(boss_spot)) {
      state_.map.at(boss_spot).type = TileType::Floor;
    }
  }

  state_.clear_entities();
  spawn_player(dungeon.player_start);
  spawn_monsters();
  spawn_items();
  if (state_.floor == kMaxFloor) {
    spawn_boss(boss_spot);
  }
  update_fov();
  state_.needs_redraw = true;
}

void Game::spawn_player(const Point& at) {
  const Entity e = state_.world.create_entity();
  state_.player = e;
  state_.positions.set(e, Position{at.x, at.y});
  state_.renderables.set(e, Renderable{'@', 1});
  state_.stats.set(e, Stats{30, 30, 5, 1, 0, 1, xp_for_level(1)});
  state_.names.set(e, Name{"Vous"});
  state_.players.set(e, PlayerTag{});
  state_.blockers.set(e, BlocksMovement{});
  state_.inventories.set(e, Inventory{});
}

void Game::spawn_monsters() {
  if (state_.rooms.size() < 2) {
    return;
  }
  std::uniform_int_distribution<int> room_dist(
      1, static_cast<int>(state_.rooms.size()) - 1);
  const int monster_count = 3 + state_.floor * 2;

  for (int i = 0; i < monster_count; ++i) {
    const Room& room = state_.rooms[static_cast<std::size_t>(room_dist(rng_))];
    std::uniform_int_distribution<int> x_dist(room.x, room.x + room.w - 1);
    std::uniform_int_distribution<int> y_dist(room.y, room.y + room.h - 1);
    const Point p{x_dist(rng_), y_dist(rng_)};
    if (!state_.map.walkable(p) || state_.blocker_at(p) != kInvalidEntity) {
      continue;
    }

    const Entity e = state_.world.create_entity();
    const bool tough = (rng_() % 5U) == 0U;
    // Trolls are tankier but never flee — rats may run when low.
    const int hp = tough ? 12 + state_.floor * 2 : 8 + state_.floor * 2;
    const int atk = tough ? 3 + state_.floor : 2 + state_.floor;
    const char glyph = tough ? 'T' : 'r';
    const std::string name = tough ? "Troll" : "Rat";

    state_.positions.set(e, Position{p.x, p.y});
    state_.renderables.set(e, Renderable{glyph, 2});
    state_.stats.set(e, Stats{hp, hp, atk, tough ? 1 : 0, 0, 1, 0});
    state_.names.set(e, Name{name});
    state_.monsters.set(
        e, MonsterTag{tough ? 25 : 10, 8, tough ? 0 : 3, false});
    state_.ais.set(e, Ai{AiState::Idle});
    state_.blockers.set(e, BlocksMovement{});
  }
}

void Game::spawn_boss(const Point& at) {
  Point spot = at;
  if (!state_.map.walkable(spot) || state_.blocker_at(spot) != kInvalidEntity) {
    if (!state_.rooms.empty()) {
      spot = state_.rooms.back().center();
    }
  }
  if (!state_.map.walkable(spot)) {
    return;
  }
  // Clear any item under the boss spawn
  const Entity blocking_item = state_.item_entity_at(spot);
  if (blocking_item != kInvalidEntity) {
    remove_entity(blocking_item);
  }

  const Entity e = state_.world.create_entity();
  const int hp = 55 + state_.floor * 8;
  const int atk = 8 + state_.floor;
  state_.positions.set(e, Position{spot.x, spot.y});
  state_.renderables.set(e, Renderable{'B', 2});
  state_.stats.set(e, Stats{hp, hp, atk, 3, 0, 1, 0});
  state_.names.set(e, Name{"Gardien Abyssal"});
  state_.monsters.set(e, MonsterTag{100, 12, 0, true});
  state_.ais.set(e, Ai{AiState::Chase});
  state_.blockers.set(e, BlocksMovement{});
  state_.log("Le Gardien Abyssal se dresse devant vous...");
}

void Game::drop_boss_loot(const Point& at) {
  Point spot = at;
  if (!state_.map.walkable(spot)) {
    return;
  }
  if (state_.item_entity_at(spot) != kInvalidEntity) {
    // Offset if something is already there
    for (const Point& n : state_.map.neighbors4(spot)) {
      if (state_.map.walkable(n) && state_.item_entity_at(n) == kInvalidEntity &&
          state_.blocker_at(n) == kInvalidEntity) {
        spot = n;
        break;
      }
    }
  }

  const Item shard = make_abyssal_shard();
  const Entity e = state_.world.create_entity();
  state_.positions.set(e, Position{spot.x, spot.y});
  state_.renderables.set(e, Renderable{shard.glyph, 3});
  state_.items.set(e, shard);
  state_.names.set(e, Name{shard.name});
  state_.log("L'Eclat Abyssal tombe au sol. Ramassez-le (G) pour gagner !");
}

void Game::spawn_items() {
  if (state_.rooms.empty()) {
    return;
  }
  std::uniform_int_distribution<int> room_dist(
      0, static_cast<int>(state_.rooms.size()) - 1);
  const int item_count = 2 + state_.floor;

  for (int i = 0; i < item_count; ++i) {
    const Room& room = state_.rooms[static_cast<std::size_t>(room_dist(rng_))];
    std::uniform_int_distribution<int> x_dist(room.x, room.x + room.w - 1);
    std::uniform_int_distribution<int> y_dist(room.y, room.y + room.h - 1);
    const Point p{x_dist(rng_), y_dist(rng_)};
    if (!state_.map.walkable(p) || state_.blocker_at(p) != kInvalidEntity) {
      continue;
    }
    if (state_.item_entity_at(p) != kInvalidEntity) {
      continue;
    }

    Item item;
    const int roll = static_cast<int>(rng_() % 3U);
    if (roll == 0) {
      item = make_health_potion();
    } else if (roll == 1) {
      item = make_strength_potion();
    } else {
      item = make_escape_scroll();
    }

    const Entity e = state_.world.create_entity();
    state_.positions.set(e, Position{p.x, p.y});
    state_.renderables.set(e, Renderable{item.glyph, 3});
    state_.items.set(e, item);
    state_.names.set(e, Name{item.name});
  }
}

void Game::update_fov() {
  const Position* pos = state_.positions.get(state_.player);
  if (pos == nullptr) {
    return;
  }
  Fov::compute(state_.map, pos->as_point(), kFovRadius);
}

void Game::check_level_up(Stats& s) {
  while (s.xp >= s.xp_to_next) {
    s.xp -= s.xp_to_next;
    ++s.level;
    s.max_hp += 5;
    s.hp = s.max_hp;
    s.attack += 1;
    s.defense += (s.level % 2 == 0) ? 1 : 0;
    s.xp_to_next = xp_for_level(s.level);
    std::ostringstream oss;
    oss << "Niveau " << s.level << " ! PV et attaque augmentes.";
    state_.log(oss.str());
  }
}

void Game::remove_entity(const Entity e) {
  state_.positions.remove(e);
  state_.renderables.remove(e);
  state_.stats.remove(e);
  state_.names.remove(e);
  state_.players.remove(e);
  state_.monsters.remove(e);
  state_.ais.remove(e);
  state_.blockers.remove(e);
  state_.items.remove(e);
  state_.inventories.remove(e);
  state_.world.destroy_entity(e);
}

void Game::kill_monster(const Entity e) {
  Point drop_at{};
  bool boss = false;
  if (const Position* p = state_.positions.get(e)) {
    drop_at = p->as_point();
  }
  if (const MonsterTag* tag = state_.monsters.get(e)) {
    boss = tag->is_boss;
  }
  remove_entity(e);
  if (boss) {
    drop_boss_loot(drop_at);
  }
  if (Stats* ps = state_.stats.get(state_.player)) {
    check_level_up(*ps);
  }
}

void Game::process_player_move(const int dx, const int dy) {
  Position* pos = state_.positions.get(state_.player);
  if (pos == nullptr) {
    return;
  }
  const Point dest{pos->x + dx, pos->y + dy};
  if (!state_.map.walkable(dest)) {
    return;
  }

  const Entity blocker = state_.blocker_at(dest);
  if (blocker != kInvalidEntity && blocker != state_.player) {
    resolve_attack(state_, state_.player, blocker);
    Stats* enemy = state_.stats.get(blocker);
    if (enemy != nullptr && enemy->hp <= 0) {
      kill_monster(blocker);
    }
  } else {
    pos->set(dest);
  }

  ++state_.turn;
  enemy_turn();
  update_fov();

  if (const Stats* ps = state_.stats.get(state_.player)) {
    if (ps->hp <= 0) {
      state_.phase = GamePhase::Dead;
      state_.log("Vous etes mort. Appuyez sur n pour une nouvelle partie.");
    }
  }
  state_.needs_redraw = true;
}

void Game::process_player_wait() {
  ++state_.turn;
  enemy_turn();
  update_fov();
  if (const Stats* ps = state_.stats.get(state_.player)) {
    if (ps->hp <= 0) {
      state_.phase = GamePhase::Dead;
      state_.log("Vous etes mort. Appuyez sur n pour une nouvelle partie.");
    }
  }
  state_.needs_redraw = true;
}

void Game::process_pickup() {
  const Position* pos = state_.positions.get(state_.player);
  Inventory* inv = state_.inventories.get(state_.player);
  if (pos == nullptr || inv == nullptr) {
    return;
  }
  const Entity item_e = state_.item_entity_at(pos->as_point());
  if (item_e == kInvalidEntity) {
    state_.log("Rien a ramasser ici.");
    state_.needs_redraw = true;
    return;
  }
  const Item* item = state_.items.get(item_e);
  if (item == nullptr) {
    return;
  }

  if (item->kind == ItemKind::AbyssalShard) {
    state_.log("L'Eclat Abyssal pulse dans vos mains...");
    state_.log("Vous avez vaincu les profondeurs. Victoire !");
    remove_entity(item_e);
    state_.phase = GamePhase::Won;
    state_.needs_redraw = true;
    return;
  }

  if (!inv->try_add(*item)) {
    state_.log("Inventaire plein.");
    state_.needs_redraw = true;
    return;
  }
  state_.log("Vous ramassez " + item->name + ".");
  remove_entity(item_e);
  ++state_.turn;
  enemy_turn();
  update_fov();
  state_.needs_redraw = true;
}

void Game::process_use_item(const std::size_t index) {
  if (!use_inventory_item(state_, index)) {
    return;
  }
  if (state_.phase == GamePhase::Won) {
    state_.needs_redraw = true;
    return;
  }
  state_.phase = GamePhase::Playing;
  ++state_.turn;
  enemy_turn();
  update_fov();
  if (const Stats* ps = state_.stats.get(state_.player)) {
    if (ps->hp <= 0) {
      state_.phase = GamePhase::Dead;
      state_.log("Vous etes mort. Appuyez sur n pour une nouvelle partie.");
    }
  }
  state_.needs_redraw = true;
}

void Game::process_stairs() {
  const Position* pos = state_.positions.get(state_.player);
  if (pos == nullptr) {
    return;
  }
  if (state_.map.at(pos->as_point()).type == TileType::StairsDown) {
    next_floor();
  } else {
    state_.log("Pas d'escalier ici. Cherchez la tuile verte (F pour descendre).");
    state_.needs_redraw = true;
  }
}

void Game::enemy_turn() {
  std::vector<Entity> monsters;
  state_.monsters.for_each([&](const Entity e, const MonsterTag&) {
    if (state_.world.alive(e)) {
      monsters.push_back(e);
    }
  });
  for (const Entity e : monsters) {
    run_ai_turn(state_, e, rng_);
  }
  // Cleanup dead monsters killed during AI (shouldn't happen often)
  std::vector<Entity> dead;
  state_.stats.for_each([&](const Entity e, const Stats& s) {
    if (s.hp <= 0 && state_.monsters.has(e)) {
      dead.push_back(e);
    }
  });
  for (const Entity e : dead) {
    kill_monster(e);
  }
}

bool Game::handle_action(const InputAction action) {
  if (action == InputAction::None) {
    return true;
  }
  if (state_.phase == GamePhase::Quit) {
    return false;
  }

  if (state_.phase == GamePhase::Dead || state_.phase == GamePhase::Won) {
    if (action == InputAction::NewGame) {
      new_game();
      return true;
    }
    if (action == InputAction::Quit) {
      state_.phase = GamePhase::Quit;
      return false;
    }
    return true;
  }

  if (state_.phase == GamePhase::Inventory) {
    if (action == InputAction::CloseInventory ||
        action == InputAction::ToggleInventory) {
      state_.phase = GamePhase::Playing;
      state_.needs_redraw = true;
      return true;
    }
    if (action >= InputAction::UseSlot0 && action <= InputAction::UseSlot9) {
      const auto index = static_cast<std::size_t>(
          static_cast<int>(action) - static_cast<int>(InputAction::UseSlot0));
      process_use_item(index);
      return true;
    }
    return true;
  }

  switch (action) {
    case InputAction::Quit:
      state_.phase = GamePhase::Quit;
      return false;
    case InputAction::MoveW:
      process_player_move(-1, 0);
      break;
    case InputAction::MoveE:
      process_player_move(1, 0);
      break;
    case InputAction::MoveN:
      process_player_move(0, -1);
      break;
    case InputAction::MoveS:
      process_player_move(0, 1);
      break;
    case InputAction::MoveNW:
      process_player_move(-1, -1);
      break;
    case InputAction::MoveNE:
      process_player_move(1, -1);
      break;
    case InputAction::MoveSW:
      process_player_move(-1, 1);
      break;
    case InputAction::MoveSE:
      process_player_move(1, 1);
      break;
    case InputAction::Wait:
      process_player_wait();
      break;
    case InputAction::Pickup:
      process_pickup();
      break;
    case InputAction::ToggleInventory:
      state_.phase = GamePhase::Inventory;
      state_.needs_redraw = true;
      break;
    case InputAction::StairsDown:
      process_stairs();
      break;
    case InputAction::Save:
      if (save_to_file(kSavePath)) {
        state_.log(std::string("Partie sauvegardee (") + kSavePath + ").");
      } else {
        state_.log("Echec de la sauvegarde.");
      }
      state_.needs_redraw = true;
      break;
    case InputAction::Load:
      if (load_from_file(kSavePath)) {
        state_.log("Partie chargee.");
      } else {
        state_.log("Impossible de charger la sauvegarde.");
      }
      state_.needs_redraw = true;
      break;
    default:
      break;
  }
  return true;
}

bool Game::save_to_file(const std::string& path) const {
  return save_game(state_, path);
}

bool Game::load_from_file(const std::string& path) {
  return load_game(state_, path, rng_);
}

}  // namespace descente
