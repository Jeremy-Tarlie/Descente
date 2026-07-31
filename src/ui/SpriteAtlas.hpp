#pragma once

#include "raylib.h"

namespace descente {

inline constexpr int kTileSize = 32;

enum class SpriteId {
  Floor,
  Wall,
  Stairs,
  Player,
  Rat,
  Troll,
  Boss,
  PotionHealth,
  PotionStrength,
  Scroll,
  AbyssalShard,
  Count,
};

class SpriteAtlas {
 public:
  SpriteAtlas();
  ~SpriteAtlas();

  SpriteAtlas(const SpriteAtlas&) = delete;
  SpriteAtlas& operator=(const SpriteAtlas&) = delete;

  [[nodiscard]] const Texture2D& get(SpriteId id) const;

 private:
  void build_all();
  [[nodiscard]] static Texture2D make_texture(Image image);

  Texture2D textures_[static_cast<int>(SpriteId::Count)]{};
  bool ready_{false};
};

}  // namespace descente
