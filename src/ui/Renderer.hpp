#pragma once

#include "game/Game.hpp"
#include "ui/SpriteAtlas.hpp"

#include "raylib.h"

#include <memory>

namespace descente {

class Renderer {
 public:
  Renderer();
  ~Renderer();

  Renderer(const Renderer&) = delete;
  Renderer& operator=(const Renderer&) = delete;

  void draw(const GameState& state);

 private:
  void update_camera(const GameState& state);
  void draw_map(const GameState& state);
  void draw_entities(const GameState& state);
  void draw_hud(const GameState& state);
  void draw_inventory(const GameState& state);
  void draw_overlay(const GameState& state);

  [[nodiscard]] static SpriteId sprite_for_monster(char glyph);
  [[nodiscard]] static SpriteId sprite_for_item(ItemKind kind);

  int width_{1280};
  int height_{720};
  Camera2D camera_{};
  std::unique_ptr<SpriteAtlas> atlas_;
  float bob_time_{0.0F};
};

}  // namespace descente
