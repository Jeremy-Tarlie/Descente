#pragma once

#include <algorithm>
#include <cstddef>
#include <optional>

#include "raylib.h"

namespace descente {

/// Shared inventory panel geometry (must stay in sync with Renderer).
struct InventoryUi {
  static constexpr int kHudHeight = 190;
  static constexpr int kBoxW = 440;
  static constexpr int kCapacity = 10;
  static constexpr int kRowH = 34;
  static constexpr int kRowsTop = 68;
  static constexpr int kBottomPad = 16;
  static constexpr int kBoxH =
      kRowsTop + kCapacity * kRowH + kBottomPad;  // 424

  [[nodiscard]] static int box_x(const int screen_w) {
    return (screen_w - kBoxW) / 2;
  }

  [[nodiscard]] static int box_y(const int screen_h) {
    const int available = screen_h - kHudHeight;
    int y = (available - kBoxH) / 2;
    if (y < 8) {
      y = 8;
    }
    return y;
  }

  [[nodiscard]] static Rectangle row_rect(const int screen_w, const int screen_h,
                                         const std::size_t index) {
    return Rectangle{
        static_cast<float>(box_x(screen_w) + 10),
        static_cast<float>(box_y(screen_h) + kRowsTop +
                           static_cast<int>(index) * kRowH),
        static_cast<float>(kBoxW - 20),
        static_cast<float>(kRowH - 4),
    };
  }

  [[nodiscard]] static std::optional<std::size_t> hit_slot(
      const int screen_w, const int screen_h, const Vector2 mouse,
      const std::size_t item_count) {
    const std::size_t n = std::min(item_count, static_cast<std::size_t>(kCapacity));
    for (std::size_t i = 0; i < n; ++i) {
      if (CheckCollisionPointRec(mouse, row_rect(screen_w, screen_h, i))) {
        return i;
      }
    }
    return std::nullopt;
  }
};

}  // namespace descente
