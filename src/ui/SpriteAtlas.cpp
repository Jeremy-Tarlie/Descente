#include "ui/SpriteAtlas.hpp"

namespace descente {

namespace {

void put_pixel(Image& image, const int x, const int y, const Color color) {
  if (x < 0 || y < 0 || x >= image.width || y >= image.height) {
    return;
  }
  ImageDrawPixel(&image, x, y, color);
}

void fill_rect(Image& image, const int x, const int y, const int w, const int h,
               const Color color) {
  for (int py = y; py < y + h; ++py) {
    for (int px = x; px < x + w; ++px) {
      put_pixel(image, px, py, color);
    }
  }
}

Image make_blank(const Color fill) {
  return GenImageColor(kTileSize, kTileSize, fill);
}

Image make_floor() {
  Image img = make_blank(Color{42, 38, 48, 255});
  for (int y = 0; y < kTileSize; ++y) {
    for (int x = 0; x < kTileSize; ++x) {
      if (((x + y * 3) % 7) == 0) {
        put_pixel(img, x, y, Color{52, 48, 58, 255});
      }
      if (((x * 5 + y) % 11) == 0) {
        put_pixel(img, x, y, Color{36, 32, 42, 255});
      }
    }
  }
  // subtle border
  for (int i = 0; i < kTileSize; ++i) {
    put_pixel(img, i, 0, Color{30, 28, 36, 255});
    put_pixel(img, 0, i, Color{30, 28, 36, 255});
  }
  return img;
}

Image make_wall() {
  Image img = make_blank(Color{78, 72, 68, 255});
  // brick pattern
  for (int row = 0; row < 4; ++row) {
    const int y0 = row * 8;
    fill_rect(img, 0, y0, kTileSize, 7, Color{96, 88, 82, 255});
    fill_rect(img, 0, y0 + 7, kTileSize, 1, Color{50, 46, 42, 255});
    const int offset = (row % 2) * 8;
    for (int col = 0; col < 5; ++col) {
      const int x = offset + col * 16 - 8;
      fill_rect(img, x, y0, 1, 7, Color{50, 46, 42, 255});
    }
  }
  // highlight top edge
  for (int x = 0; x < kTileSize; ++x) {
    put_pixel(img, x, 0, Color{130, 120, 110, 255});
  }
  return img;
}

Image make_stairs() {
  Image img = make_floor();
  fill_rect(img, 6, 6, 20, 20, Color{28, 90, 70, 255});
  fill_rect(img, 8, 8, 16, 4, Color{60, 160, 120, 255});
  fill_rect(img, 10, 14, 12, 4, Color{50, 140, 105, 255});
  fill_rect(img, 12, 20, 8, 4, Color{40, 120, 90, 255});
  // arrow down
  put_pixel(img, 15, 11, Color{220, 255, 220, 255});
  put_pixel(img, 16, 12, Color{220, 255, 220, 255});
  put_pixel(img, 17, 11, Color{220, 255, 220, 255});
  put_pixel(img, 16, 13, Color{220, 255, 220, 255});
  return img;
}

Image make_player() {
  Image img = GenImageColor(kTileSize, kTileSize, BLANK);
  // shadow
  fill_rect(img, 10, 26, 12, 3, Color{0, 0, 0, 80});
  // body cloak
  fill_rect(img, 10, 14, 12, 12, Color{210, 160, 50, 255});
  fill_rect(img, 8, 16, 16, 8, Color{190, 140, 40, 255});
  // head
  fill_rect(img, 12, 6, 8, 8, Color{240, 210, 170, 255});
  // eyes
  put_pixel(img, 14, 9, Color{30, 30, 40, 255});
  put_pixel(img, 17, 9, Color{30, 30, 40, 255});
  // hair
  fill_rect(img, 12, 5, 8, 2, Color{60, 40, 30, 255});
  // boots
  fill_rect(img, 10, 25, 5, 3, Color{40, 30, 25, 255});
  fill_rect(img, 17, 25, 5, 3, Color{40, 30, 25, 255});
  return img;
}

Image make_rat() {
  Image img = GenImageColor(kTileSize, kTileSize, BLANK);
  fill_rect(img, 10, 26, 12, 2, Color{0, 0, 0, 70});
  fill_rect(img, 8, 14, 16, 10, Color{140, 110, 90, 255});
  fill_rect(img, 6, 16, 4, 4, Color{120, 90, 70, 255});   // head
  fill_rect(img, 22, 18, 6, 2, Color{160, 100, 100, 255});  // tail
  put_pixel(img, 7, 17, Color{20, 20, 20, 255});
  put_pixel(img, 8, 16, Color{220, 60, 60, 255});
  // ears
  put_pixel(img, 6, 14, Color{160, 120, 100, 255});
  put_pixel(img, 9, 14, Color{160, 120, 100, 255});
  return img;
}

Image make_troll() {
  Image img = GenImageColor(kTileSize, kTileSize, BLANK);
  fill_rect(img, 8, 27, 16, 3, Color{0, 0, 0, 80});
  fill_rect(img, 8, 10, 16, 16, Color{70, 120, 70, 255});
  fill_rect(img, 10, 4, 12, 10, Color{90, 140, 80, 255});
  put_pixel(img, 13, 8, Color{255, 220, 40, 255});
  put_pixel(img, 18, 8, Color{255, 220, 40, 255});
  fill_rect(img, 13, 12, 6, 2, Color{30, 50, 30, 255});
  // club
  fill_rect(img, 24, 8, 4, 14, Color{110, 80, 50, 255});
  fill_rect(img, 22, 6, 8, 5, Color{130, 95, 60, 255});
  return img;
}

Image make_boss() {
  Image img = GenImageColor(kTileSize, kTileSize, BLANK);
  fill_rect(img, 6, 27, 20, 3, Color{0, 0, 0, 90});
  fill_rect(img, 7, 8, 18, 18, Color{90, 40, 120, 255});
  fill_rect(img, 9, 3, 14, 10, Color{120, 50, 150, 255});
  fill_rect(img, 11, 1, 10, 4, Color{40, 20, 60, 255});  // horns base
  put_pixel(img, 10, 0, Color{200, 180, 80, 255});
  put_pixel(img, 21, 0, Color{200, 180, 80, 255});
  put_pixel(img, 12, 6, Color{255, 60, 60, 255});
  put_pixel(img, 19, 6, Color{255, 60, 60, 255});
  fill_rect(img, 13, 11, 6, 2, Color{20, 10, 30, 255});
  fill_rect(img, 4, 12, 4, 10, Color{60, 30, 80, 255});
  fill_rect(img, 24, 12, 4, 10, Color{60, 30, 80, 255});
  return img;
}

Image make_potion(const Color liquid) {
  Image img = GenImageColor(kTileSize, kTileSize, BLANK);
  fill_rect(img, 13, 6, 6, 3, Color{200, 200, 210, 255});  // cork
  fill_rect(img, 12, 9, 8, 3, Color{180, 200, 210, 255});  // neck
  fill_rect(img, 10, 12, 12, 14, Color{160, 190, 200, 180});
  fill_rect(img, 11, 16, 10, 8, liquid);
  fill_rect(img, 12, 14, 2, 6, Color{255, 255, 255, 100});  // shine
  return img;
}

Image make_scroll() {
  Image img = GenImageColor(kTileSize, kTileSize, BLANK);
  fill_rect(img, 8, 8, 16, 18, Color{230, 220, 190, 255});
  fill_rect(img, 8, 8, 16, 2, Color{200, 180, 140, 255});
  fill_rect(img, 8, 24, 16, 2, Color{200, 180, 140, 255});
  for (int i = 0; i < 4; ++i) {
    fill_rect(img, 11, 12 + i * 3, 10, 1, Color{80, 50, 120, 255});
  }
  return img;
}

Image make_shard() {
  Image img = GenImageColor(kTileSize, kTileSize, BLANK);
  // glowing diamond
  for (int i = 0; i < 8; ++i) {
    fill_rect(img, 16 - i, 8 + i, 1 + i * 2, 1, Color{120, 80, 255, 255});
  }
  for (int i = 0; i < 8; ++i) {
    fill_rect(img, 9 + i, 16 + i, 15 - i * 2, 1, Color{180, 140, 255, 255});
  }
  fill_rect(img, 14, 12, 4, 4, Color{240, 230, 255, 255});
  put_pixel(img, 16, 6, Color{255, 255, 255, 255});
  return img;
}

}  // namespace

SpriteAtlas::SpriteAtlas() { build_all(); }

SpriteAtlas::~SpriteAtlas() {
  if (!ready_) {
    return;
  }
  for (int i = 0; i < static_cast<int>(SpriteId::Count); ++i) {
    if (textures_[i].id != 0) {
      UnloadTexture(textures_[i]);
    }
  }
  ready_ = false;
}

Texture2D SpriteAtlas::make_texture(Image image) {
  const Texture2D tex = LoadTextureFromImage(image);
  UnloadImage(image);
  SetTextureFilter(tex, TEXTURE_FILTER_POINT);
  return tex;
}

void SpriteAtlas::build_all() {
  textures_[static_cast<int>(SpriteId::Floor)] = make_texture(make_floor());
  textures_[static_cast<int>(SpriteId::Wall)] = make_texture(make_wall());
  textures_[static_cast<int>(SpriteId::Stairs)] = make_texture(make_stairs());
  textures_[static_cast<int>(SpriteId::Player)] = make_texture(make_player());
  textures_[static_cast<int>(SpriteId::Rat)] = make_texture(make_rat());
  textures_[static_cast<int>(SpriteId::Troll)] = make_texture(make_troll());
  textures_[static_cast<int>(SpriteId::Boss)] = make_texture(make_boss());
  textures_[static_cast<int>(SpriteId::PotionHealth)] =
      make_texture(make_potion(Color{200, 50, 70, 255}));
  textures_[static_cast<int>(SpriteId::PotionStrength)] =
      make_texture(make_potion(Color{220, 140, 40, 255}));
  textures_[static_cast<int>(SpriteId::Scroll)] = make_texture(make_scroll());
  textures_[static_cast<int>(SpriteId::AbyssalShard)] =
      make_texture(make_shard());
  ready_ = true;
}

const Texture2D& SpriteAtlas::get(const SpriteId id) const {
  return textures_[static_cast<int>(id)];
}

}  // namespace descente
