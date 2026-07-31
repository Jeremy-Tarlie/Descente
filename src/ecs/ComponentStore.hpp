#pragma once

#include "ecs/Entity.hpp"

#include <optional>
#include <utility>
#include <vector>

namespace descente {

/// Contiguous component store keyed by dense entity indices.
/// Entities are expected to be consecutive IDs starting at 0.
template <typename T>
class ComponentStore {
 public:
  void resize(const Entity capacity) {
    data_.resize(static_cast<std::size_t>(capacity));
  }

  [[nodiscard]] std::size_t size() const noexcept { return data_.size(); }

  void clear() { data_.clear(); }

  void set(const Entity entity, T value) {
    ensure_capacity(entity);
    data_[static_cast<std::size_t>(entity)] = std::move(value);
  }

  void remove(const Entity entity) {
    if (entity < data_.size()) {
      data_[static_cast<std::size_t>(entity)].reset();
    }
  }

  [[nodiscard]] bool has(const Entity entity) const noexcept {
    return entity < data_.size() && data_[static_cast<std::size_t>(entity)].has_value();
  }

  [[nodiscard]] T* get(const Entity entity) {
    if (!has(entity)) {
      return nullptr;
    }
    return &(*data_[static_cast<std::size_t>(entity)]);
  }

  [[nodiscard]] const T* get(const Entity entity) const {
    if (!has(entity)) {
      return nullptr;
    }
    return &(*data_[static_cast<std::size_t>(entity)]);
  }

  template <typename Fn>
  void for_each(Fn&& fn) {
    for (Entity e = 0; e < static_cast<Entity>(data_.size()); ++e) {
      if (data_[static_cast<std::size_t>(e)].has_value()) {
        fn(e, *data_[static_cast<std::size_t>(e)]);
      }
    }
  }

  template <typename Fn>
  void for_each(Fn&& fn) const {
    for (Entity e = 0; e < static_cast<Entity>(data_.size()); ++e) {
      if (data_[static_cast<std::size_t>(e)].has_value()) {
        fn(e, *data_[static_cast<std::size_t>(e)]);
      }
    }
  }

 private:
  void ensure_capacity(const Entity entity) {
    if (entity >= data_.size()) {
      data_.resize(static_cast<std::size_t>(entity) + 1U);
    }
  }

  std::vector<std::optional<T>> data_;
};

}  // namespace descente
