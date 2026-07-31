#include "game/Game.hpp"
#include "ui/InputPoll.hpp"
#include "ui/Renderer.hpp"

#include <exception>
#include <iostream>

int main() {
  try {
    descente::Renderer renderer;
    descente::Game game;
    game.new_game();

    bool running = true;
    while (running && !WindowShouldClose()) {
      const descente::InputAction action = descente::poll_input(game.state());
      if (action != descente::InputAction::None) {
        running = game.handle_action(action);
      }
      if (game.state().phase == descente::GamePhase::Quit) {
        running = false;
      }
      renderer.draw(game.state());
    }
  } catch (const std::exception& ex) {
    std::cerr << "Erreur fatale: " << ex.what() << '\n';
    return 1;
  }
  return 0;
}
