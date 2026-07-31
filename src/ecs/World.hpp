#pragma once

#include "ecs/Entity.hpp"

#include <vector>

namespace descente {

/// Manages dense entity lifetimes. Component data lives in typed stores
/// owned by GameState (contiguous vectors keyed by Entity id).
class World {
 public:
  Entity create_entity() {
    if (!free_list_.empty()) {
      const Entity id = free_list_.back();
      free_list_.pop_back();
      alive_[static_cast<std::size_t>(id)] = true;
      return id;
    }
    const Entity id = static_cast<Entity>(alive_.size());
    alive_.push_back(true);
    return id;
  }

  void destroy_entity(const Entity entity) {
    if (entity >= alive_.size() || !alive_[static_cast<std::size_t>(entity)]) {
      return;
    }
    alive_[static_cast<std::size_t>(entity)] = false;
    free_list_.push_back(entity);
  }

  [[nodiscard]] bool alive(const Entity entity) const noexcept {
    return entity < alive_.size() && alive_[static_cast<std::size_t>(entity)];
  }

  [[nodiscard]] Entity capacity() const noexcept {
    return static_cast<Entity>(alive_.size());
  }

  void clear() {
    alive_.clear();
    free_list_.clear();
  }

  template <typename Fn>
  void for_each_alive(Fn&& fn) const {
    for (Entity e = 0; e < static_cast<Entity>(alive_.size()); ++e) {
      if (alive_[static_cast<std::size_t>(e)]) {
        fn(e);
      }
    }
  }

 private:
  std::vector<bool> alive_;
  std::vector<Entity> free_list_;
};

}  // namespace descente
