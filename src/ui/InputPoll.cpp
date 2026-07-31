#include "ui/InputPoll.hpp"

#include "ui/InventoryUi.hpp"

#include "raylib.h"

namespace descente {

namespace {

constexpr float kMoveInitialDelay = 0.18F;
constexpr float kMoveRepeatInterval = 0.08F;

InputAction slot_action(const std::size_t index) {
  switch (index) {
    case 0:
      return InputAction::UseSlot0;
    case 1:
      return InputAction::UseSlot1;
    case 2:
      return InputAction::UseSlot2;
    case 3:
      return InputAction::UseSlot3;
    case 4:
      return InputAction::UseSlot4;
    case 5:
      return InputAction::UseSlot5;
    case 6:
      return InputAction::UseSlot6;
    case 7:
      return InputAction::UseSlot7;
    case 8:
      return InputAction::UseSlot8;
    case 9:
      return InputAction::UseSlot9;
    default:
      return InputAction::None;
  }
}

InputAction slot_from_digit_char(const int ch) {
  if (ch >= '1' && ch <= '9') {
    return slot_action(static_cast<std::size_t>(ch - '1'));
  }
  if (ch == '0') {
    return slot_action(9);
  }
  return InputAction::None;
}

InputAction held_move_action(const bool shift) {
  // Diagonals first so Y/U/B/N win over cardinals when both held.
  if (IsKeyDown(KEY_Y)) {
    return InputAction::MoveNW;
  }
  if (IsKeyDown(KEY_U)) {
    return InputAction::MoveNE;
  }
  if (IsKeyDown(KEY_B)) {
    return InputAction::MoveSW;
  }
  if (IsKeyDown(KEY_N)) {
    return InputAction::MoveSE;
  }

  if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A) || IsKeyDown(KEY_H)) {
    return InputAction::MoveW;
  }
  if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D) || IsKeyDown(KEY_L)) {
    return InputAction::MoveE;
  }
  if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W) || IsKeyDown(KEY_K)) {
    return InputAction::MoveN;
  }
  if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_J) || (IsKeyDown(KEY_S) && !shift)) {
    return InputAction::MoveS;
  }
  return InputAction::None;
}

InputAction poll_move_with_repeat(const bool shift) {
  static InputAction last_move = InputAction::None;
  static float hold_time = 0.0F;
  static bool repeating = false;

  const InputAction current = held_move_action(shift);
  if (current == InputAction::None) {
    last_move = InputAction::None;
    hold_time = 0.0F;
    repeating = false;
    return InputAction::None;
  }

  if (current != last_move) {
    last_move = current;
    hold_time = 0.0F;
    repeating = false;
    return current;
  }

  hold_time += GetFrameTime();
  const float threshold = repeating ? kMoveRepeatInterval : kMoveInitialDelay;
  if (hold_time >= threshold) {
    hold_time = 0.0F;
    repeating = true;
    return current;
  }
  return InputAction::None;
}

}  // namespace

InputAction poll_input(const GameState& state) {
  const GamePhase phase = state.phase;

  if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_Q)) {
    if (phase == GamePhase::Inventory) {
      return InputAction::CloseInventory;
    }
    return InputAction::Quit;
  }

  if (phase == GamePhase::Dead || phase == GamePhase::Won) {
    if (IsKeyPressed(KEY_N)) {
      return InputAction::NewGame;
    }
    return InputAction::None;
  }

  if (phase == GamePhase::Inventory) {
    if (IsKeyPressed(KEY_I)) {
      return InputAction::ToggleInventory;
    }

    int ch = GetCharPressed();
    while (ch > 0) {
      const InputAction from_char = slot_from_digit_char(ch);
      if (from_char != InputAction::None) {
        return from_char;
      }
      ch = GetCharPressed();
    }

    if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_KP_1)) {
      return InputAction::UseSlot0;
    }
    if (IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2)) {
      return InputAction::UseSlot1;
    }
    if (IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_KP_3)) {
      return InputAction::UseSlot2;
    }
    if (IsKeyPressed(KEY_FOUR) || IsKeyPressed(KEY_KP_4)) {
      return InputAction::UseSlot3;
    }
    if (IsKeyPressed(KEY_FIVE) || IsKeyPressed(KEY_KP_5)) {
      return InputAction::UseSlot4;
    }
    if (IsKeyPressed(KEY_SIX) || IsKeyPressed(KEY_KP_6)) {
      return InputAction::UseSlot5;
    }
    if (IsKeyPressed(KEY_SEVEN) || IsKeyPressed(KEY_KP_7)) {
      return InputAction::UseSlot6;
    }
    if (IsKeyPressed(KEY_EIGHT) || IsKeyPressed(KEY_KP_8)) {
      return InputAction::UseSlot7;
    }
    if (IsKeyPressed(KEY_NINE) || IsKeyPressed(KEY_KP_9)) {
      return InputAction::UseSlot8;
    }
    if (IsKeyPressed(KEY_ZERO) || IsKeyPressed(KEY_KP_0)) {
      return InputAction::UseSlot9;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      const Inventory* inv = state.inventories.get(state.player);
      const std::size_t count = (inv != nullptr) ? inv->items.size() : 0;
      const auto hit = InventoryUi::hit_slot(GetScreenWidth(), GetScreenHeight(),
                                            GetMousePosition(), count);
      if (hit.has_value()) {
        return slot_action(*hit);
      }
    }

    return InputAction::None;
  }

  const bool shift =
      IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

  if (IsKeyPressed(KEY_S) && shift) {
    return InputAction::Save;
  }
  if (IsKeyPressed(KEY_C) || (IsKeyPressed(KEY_L) && shift)) {
    return InputAction::Load;
  }

  if (IsKeyPressed(KEY_F) || IsKeyPressed(KEY_PAGE_DOWN) ||
      (IsKeyPressed(KEY_PERIOD) && shift)) {
    return InputAction::StairsDown;
  }

  // Movement: first press instant, then auto-repeat while held.
  const InputAction move = poll_move_with_repeat(shift);
  if (move != InputAction::None) {
    return move;
  }

  if ((IsKeyPressed(KEY_PERIOD) && !shift) || IsKeyPressed(KEY_SPACE)) {
    return InputAction::Wait;
  }
  if (IsKeyPressed(KEY_G) || IsKeyPressed(KEY_COMMA)) {
    return InputAction::Pickup;
  }
  if (IsKeyPressed(KEY_I)) {
    return InputAction::ToggleInventory;
  }

  return InputAction::None;
}

}  // namespace descente
