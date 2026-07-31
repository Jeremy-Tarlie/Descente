#pragma once

#include "ecs/Entity.hpp"
#include "map/Map.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace descente {

enum class AiState : std::uint8_t {
  Idle,
  Chase,
  Flee,
};

enum class ItemKind : std::uint8_t {
  HealthPotion,
  StrengthPotion,
  EscapeScroll,
  AbyssalShard,
};

struct Position {
  int x{0};
  int y{0};

  [[nodiscard]] Point as_point() const noexcept { return Point{x, y}; }

  void set(const Point& p) noexcept {
    x = p.x;
    y = p.y;
  }
};

struct Renderable {
  char glyph{'?'};
  int color_pair{0};
};

struct Stats {
  int hp{10};
  int max_hp{10};
  int attack{2};
  int defense{0};
  int xp{0};
  int level{1};
  int xp_to_next{20};
};

struct Name {
  std::string value;
};

struct PlayerTag {};

struct MonsterTag {
  int xp_reward{10};
  int detection_range{8};
  int flee_hp_threshold{3};
  bool is_boss{false};
};

struct Ai {
  AiState state{AiState::Idle};
};

struct BlocksMovement {};

struct Item {
  ItemKind kind{ItemKind::HealthPotion};
  std::string name;
  char glyph{'!'};
};

struct Inventory {
  static constexpr std::size_t kCapacity = 10;
  std::vector<Item> items;

  [[nodiscard]] bool full() const noexcept { return items.size() >= kCapacity; }

  bool try_add(Item item) {
    if (full()) {
      return false;
    }
    items.push_back(std::move(item));
    return true;
  }
};

inline Item make_health_potion() {
  return Item{ItemKind::HealthPotion, "Potion de soin", '!'};
}

inline Item make_strength_potion() {
  return Item{ItemKind::StrengthPotion, "Potion de force", '!'};
}

inline Item make_escape_scroll() {
  return Item{ItemKind::EscapeScroll, "Parchemin de fuite", '?'};
}

inline Item make_abyssal_shard() {
  return Item{ItemKind::AbyssalShard, "Eclat Abyssal", '*'};
}

inline int xp_for_level(const int level) {
  return 20 + (level - 1) * 15;
}

}  // namespace descente
