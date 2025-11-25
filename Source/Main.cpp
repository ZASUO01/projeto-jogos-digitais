//
// Created by pedro-souza on 23/11/2025.
//
#include "Game.h"

int main() {
    Game game;
    if (bool success = game.Initialize()) {
        game.RunLoop();
    }
    game.Shutdown();

    return 0;
}