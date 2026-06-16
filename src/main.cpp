#include "Game.hpp"
#include <memory>

int main() {
    auto game = std::make_unique<Game>(GAME_WIDTH, GAME_HEIGHT, "voxl");
    game->Run();
}