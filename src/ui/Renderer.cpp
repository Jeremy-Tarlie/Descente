#include "ui/Renderer.hpp"

#include "ui/InventoryUi.hpp"

#include <cmath>
#include <cstdio>
#include <string>

namespace descente {

namespace {

constexpr Color kBg = {12, 10, 16, 255};
constexpr Color kHudBg = {0, 0, 0, 200};
constexpr Color kExploredTint = {90, 100, 140, 255};

}  // namespace

Renderer::Renderer() {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
  InitWindow(width_, height_, "Descente — Roguelike 2D");
  SetTargetFPS(60);
  SetExitKey(KEY_NULL);  // we handle Quit ourselves

  atlas_ = std::make_unique<SpriteAtlas>();

  camera_.zoom = 1.6F;
  camera_.rotation = 0.0F;
  camera_.offset = Vector2{
      static_cast<float>(width_) * 0.5F,
      static_cast<float>(height_ - InventoryUi::kHudHeight) * 0.5F};
  camera_.target = Vector2{0.0F, 0.0F};
}

Renderer::~Renderer() {
  atlas_.reset();
  if (IsWindowReady()) {
    CloseWindow();
  }
}

SpriteId Renderer::sprite_for_monster(const char glyph) {
  if (glyph == 'T') {
    return SpriteId::Troll;
  }
  if (glyph == 'B') {
    return SpriteId::Boss;
  }
  return SpriteId::Rat;
}

SpriteId Renderer::sprite_for_item(const ItemKind kind) {
  switch (kind) {
    case ItemKind::HealthPotion:
      return SpriteId::PotionHealth;
    case ItemKind::StrengthPotion:
      return SpriteId::PotionStrength;
    case ItemKind::EscapeScroll:
      return SpriteId::Scroll;
    case ItemKind::AbyssalShard:
      return SpriteId::AbyssalShard;
  }
  return SpriteId::Scroll;
}

void Renderer::update_camera(const GameState& state) {
  width_ = GetScreenWidth();
  height_ = GetScreenHeight();
  camera_.offset = Vector2{
      static_cast<float>(width_) * 0.5F,
      static_cast<float>(height_ - InventoryUi::kHudHeight) * 0.5F};

  if (const Position* pos = state.positions.get(state.player)) {
    const Vector2 desired{
        static_cast<float>(pos->x * kTileSize + kTileSize / 2),
        static_cast<float>(pos->y * kTileSize + kTileSize / 2),
    };
    camera_.target.x += (desired.x - camera_.target.x) * 0.18F;
    camera_.target.y += (desired.y - camera_.target.y) * 0.18F;
  }
}

void Renderer::draw_map(const GameState& state) {
  for (int y = 0; y < state.map.height(); ++y) {
    for (int x = 0; x < state.map.width(); ++x) {
      const Tile& tile = state.map.at(x, y);
      if (!tile.visible && !tile.explored) {
        continue;
      }

      SpriteId sid = SpriteId::Floor;
      if (tile.type == TileType::Wall) {
        sid = SpriteId::Wall;
      } else if (tile.type == TileType::StairsDown ||
                 tile.type == TileType::StairsUp) {
        sid = SpriteId::Stairs;
      }

      const Texture2D& tex = atlas_->get(sid);
      const Vector2 pos{static_cast<float>(x * kTileSize),
                        static_cast<float>(y * kTileSize)};
      const Color tint = tile.visible ? WHITE : kExploredTint;
      DrawTextureEx(tex, pos, 0.0F, 1.0F, tint);
    }
  }
}

void Renderer::draw_entities(const GameState& state) {
  bob_time_ += GetFrameTime();
  const float bob = std::sin(bob_time_ * 4.0F) * 1.5F;

  state.items.for_each([&](const Entity e, const Item& item) {
    if (!state.world.alive(e) || state.blockers.has(e)) {
      return;
    }
    const Position* pos = state.positions.get(e);
    if (pos == nullptr || !state.map.at(pos->as_point()).visible) {
      return;
    }
    const Texture2D& tex = atlas_->get(sprite_for_item(item.kind));
    const Vector2 p{static_cast<float>(pos->x * kTileSize),
                    static_cast<float>(pos->y * kTileSize)};
    DrawTextureEx(tex, p, 0.0F, 1.0F, WHITE);
  });

  state.monsters.for_each([&](const Entity e, const MonsterTag&) {
    if (!state.world.alive(e)) {
      return;
    }
    const Position* pos = state.positions.get(e);
    const Renderable* rend = state.renderables.get(e);
    const Stats* stats = state.stats.get(e);
    if (pos == nullptr || rend == nullptr ||
        !state.map.at(pos->as_point()).visible) {
      return;
    }
    const Texture2D& tex = atlas_->get(sprite_for_monster(rend->glyph));
    const Vector2 p{static_cast<float>(pos->x * kTileSize),
                    static_cast<float>(pos->y * kTileSize)};
    DrawTextureEx(tex, p, 0.0F, 1.0F, WHITE);

    if (stats != nullptr && stats->hp < stats->max_hp) {
      const float ratio =
          static_cast<float>(stats->hp) / static_cast<float>(stats->max_hp);
      const float bar_w = static_cast<float>(kTileSize - 4);
      DrawRectangle(pos->x * kTileSize + 2, pos->y * kTileSize - 4,
                    static_cast<int>(bar_w), 3, Color{40, 20, 20, 200});
      DrawRectangle(pos->x * kTileSize + 2, pos->y * kTileSize - 4,
                    static_cast<int>(bar_w * ratio), 3, Color{200, 50, 50, 255});
    }
  });

  if (const Position* pos = state.positions.get(state.player)) {
    const Texture2D& tex = atlas_->get(SpriteId::Player);
    const Vector2 p{static_cast<float>(pos->x * kTileSize),
                    static_cast<float>(pos->y * kTileSize) + bob};
    DrawTextureEx(tex, p, 0.0F, 1.0F, WHITE);
  }
}

void Renderer::draw_hud(const GameState& state) {
  const int hud_h = InventoryUi::kHudHeight;
  const int y0 = height_ - hud_h;
  DrawRectangle(0, y0, width_, hud_h, kHudBg);
  DrawRectangle(0, y0, width_, 2, Color{180, 140, 60, 255});

  const Stats* stats = state.stats.get(state.player);
  char line1[256];
  if (stats != nullptr) {
    std::snprintf(
        line1, sizeof(line1),
        "Etage %d/%d   PV %d/%d   ATK %d  DEF %d   Niv %d  XP %d/%d   Tour %d",
        state.floor, 5, stats->hp, stats->max_hp, stats->attack, stats->defense,
        stats->level, stats->xp, stats->xp_to_next, state.turn);

    const float ratio =
        static_cast<float>(stats->hp) / static_cast<float>(stats->max_hp);
    DrawRectangle(16, y0 + 34, 240, 12, Color{50, 20, 20, 255});
    DrawRectangle(16, y0 + 34, static_cast<int>(240.0F * ratio), 12,
                  Color{200, 55, 55, 255});
    DrawRectangleLines(16, y0 + 34, 240, 12, Color{220, 180, 100, 255});
  } else {
    std::snprintf(line1, sizeof(line1), "Etage %d", state.floor);
  }

  DrawText(line1, 16, y0 + 8, 18, Color{240, 220, 160, 255});
  DrawText("WASD bouger  |  G ramasser  |  I inventaire  |  F descendre", 16,
           y0 + 52, 15, Color{170, 170, 180, 255});
  DrawText("But : vaincre le Gardien a l'etage 5 et ramasser l'Eclat Abyssal",
           16, y0 + 70, 15, Color{190, 170, 220, 255});

  int msg_y = y0 + 94;
  constexpr int kMsgLine = 16;
  for (const std::string& msg : state.messages) {
    if (msg_y + kMsgLine > height_ - 6) {
      break;
    }
    DrawText(msg.c_str(), 16, msg_y, 15, Color{210, 210, 220, 255});
    msg_y += kMsgLine;
  }
}

void Renderer::draw_inventory(const GameState& state) {
  const Inventory* inv = state.inventories.get(state.player);
  const int box_x = InventoryUi::box_x(width_);
  const int box_y = InventoryUi::box_y(height_);

  DrawRectangle(0, 0, width_, height_ - InventoryUi::kHudHeight,
                Color{0, 0, 0, 140});
  DrawRectangle(box_x, box_y, InventoryUi::kBoxW, InventoryUi::kBoxH,
                Color{24, 22, 30, 240});
  DrawRectangleLines(box_x, box_y, InventoryUi::kBoxW, InventoryUi::kBoxH,
                     Color{200, 160, 70, 255});
  DrawText("Inventaire", box_x + 16, box_y + 10, 22, Color{240, 210, 120, 255});
  DrawText("Cliquez un objet pour l'utiliser   |   Esc pour fermer", box_x + 16,
           box_y + 40, 15, Color{170, 170, 180, 255});

  if (inv == nullptr || inv->items.empty()) {
    DrawText("(vide — ramassez un objet avec G)", box_x + 16, box_y + 100, 18,
             Color{140, 140, 150, 255});
    return;
  }

  const Vector2 mouse = GetMousePosition();
  for (std::size_t i = 0; i < inv->items.size(); ++i) {
    if (i >= static_cast<std::size_t>(InventoryUi::kCapacity)) {
      break;
    }
    const Rectangle row = InventoryUi::row_rect(width_, height_, i);
    const bool hovered = CheckCollisionPointRec(mouse, row);
    DrawRectangleRec(row, hovered ? Color{60, 50, 35, 255}
                                  : Color{35, 32, 42, 255});
    DrawRectangleLinesEx(row, 1.0F, hovered ? Color{230, 190, 90, 255}
                                            : Color{80, 75, 90, 255});

    const char key =
        (i < 9) ? static_cast<char>('1' + static_cast<int>(i)) : '0';
    char line[128];
    std::snprintf(line, sizeof(line), "[%c]  %s", key, inv->items[i].name.c_str());
    const Texture2D& tex = atlas_->get(sprite_for_item(inv->items[i].kind));
    DrawTextureEx(tex, Vector2{row.x + 4.0F, row.y + 1.0F}, 0.0F, 0.8F, WHITE);
    DrawText(line, static_cast<int>(row.x) + 40, static_cast<int>(row.y) + 7, 17,
             Color{230, 230, 235, 255});
  }
}

void Renderer::draw_overlay(const GameState& state) {
  if (state.phase != GamePhase::Dead && state.phase != GamePhase::Won) {
    return;
  }
  DrawRectangle(0, 0, width_, height_, Color{0, 0, 0, 160});
  const char* title =
      state.phase == GamePhase::Dead ? "VOUS ETES MORT" : "VICTOIRE";
  const Color title_color =
      state.phase == GamePhase::Dead ? Color{220, 70, 70, 255}
                                     : Color{80, 220, 140, 255};
  const int tw = MeasureText(title, 42);
  DrawText(title, (width_ - tw) / 2, height_ / 2 - 60, 42, title_color);
  if (state.phase == GamePhase::Won) {
    const char* sub = "L'Eclat Abyssal est a vous.";
    const int sw = MeasureText(sub, 22);
    DrawText(sub, (width_ - sw) / 2, height_ / 2 - 10, 22,
             Color{200, 180, 255, 255});
  }
  const char* hint = "N : nouvelle partie    Esc : quitter";
  const int hw = MeasureText(hint, 20);
  DrawText(hint, (width_ - hw) / 2, height_ / 2 + 30, 20,
           Color{220, 220, 230, 255});
}

void Renderer::draw(const GameState& state) {
  update_camera(state);

  BeginDrawing();
  ClearBackground(kBg);

  BeginMode2D(camera_);
  draw_map(state);
  draw_entities(state);
  EndMode2D();

  draw_hud(state);
  if (state.phase == GamePhase::Inventory) {
    draw_inventory(state);
  }
  draw_overlay(state);

  EndDrawing();
}

}  // namespace descente
