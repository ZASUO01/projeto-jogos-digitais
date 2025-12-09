#define SDL_MAIN_HANDLED
#include "Game.h"

int main() {
    Game game;
    if (bool success = game.Initialize()) {
        game.RunLoop();
    }
    game.Shutdown();
    return 0;
}