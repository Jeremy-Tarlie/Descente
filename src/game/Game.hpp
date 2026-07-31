#pragma once

#include "ecs/ComponentStore.hpp"
#include "ecs/World.hpp"
#include "game/Components.hpp"
#include "game/Input.hpp"
#include "map/BspGenerator.hpp"
#include "map/Map.hpp"

#include <deque>
#include <random>
#include <string>

namespace descente {

enum class GamePhase {
  Playing,
  Inventory,
  Dead,
  Won,
  Quit,
};

struct GameState {
  World world;
  Map map;
  std::vector<Room> rooms;

  ComponentStore<Position> positions;
  ComponentStore<Renderable> renderables;
  ComponentStore<Stats> stats;
  ComponentStore<Name> names;
  ComponentStore<PlayerTag> players;
  ComponentStore<MonsterTag> monsters;
  ComponentStore<Ai> ais;
  ComponentStore<BlocksMovement> blockers;
  ComponentStore<Item> items;
  ComponentStore<Inventory> inventories;

  Entity player{kInvalidEntity};
  int floor{1};
  int turn{0};
  std::uint32_t seed{0};
  GamePhase phase{GamePhase::Playing};
  std::deque<std::string> messages;
  bool needs_redraw{true};

  void log(std::string message) {
    messages.push_back(std::move(message));
    while (messages.size() > 5) {
      messages.pop_front();
    }
  }

  void clear_entities() {
    world.clear();
    positions.clear();
    renderables.clear();
    stats.clear();
    names.clear();
    players.clear();
    monsters.clear();
    ais.clear();
    blockers.clear();
    items.clear();
    inventories.clear();
    player = kInvalidEntity;
  }

  [[nodiscard]] Entity blocker_at(const Point& p) const {
    Entity found = kInvalidEntity;
    positions.for_each([&](const Entity e, const Position& pos) {
      if (pos.x == p.x && pos.y == p.y && world.alive(e) && blockers.has(e)) {
        found = e;
      }
    });
    return found;
  }

  [[nodiscard]] Entity item_entity_at(const Point& p) const {
    Entity found = kInvalidEntity;
    positions.for_each([&](const Entity e, const Position& pos) {
      if (pos.x == p.x && pos.y == p.y && world.alive(e) && items.has(e) &&
          !blockers.has(e)) {
        found = e;
      }
    });
    return found;
  }
};

class Game {
 public:
  Game();

  void new_game(std::uint32_t seed = 0);
  void next_floor();
  [[nodiscard]] bool handle_action(InputAction action);
  void update_fov();

  [[nodiscard]] const GameState& state() const noexcept { return state_; }
  [[nodiscard]] GameState& state() noexcept { return state_; }

  bool save_to_file(const std::string& path) const;
  bool load_from_file(const std::string& path);

 private:
  void generate_floor();
  void spawn_player(const Point& at);
  void spawn_monsters();
  void spawn_items();
  void spawn_boss(const Point& at);
  void drop_boss_loot(const Point& at);
  void process_player_move(int dx, int dy);
  void process_player_wait();
  void process_pickup();
  void process_use_item(std::size_t index);
  void process_stairs();
  void enemy_turn();
  void remove_entity(Entity e);
  void kill_monster(Entity e);
  void check_level_up(Stats& s);

  GameState state_;
  std::mt19937 rng_;
  BspConfig bsp_config_{};
};

}  // namespace descente
