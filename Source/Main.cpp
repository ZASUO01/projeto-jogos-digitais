#define SDL_MAIN_HANDLED
#include "Game.h"
#include "../Network/Platforms.h"

int main() {
    networkingInit();

    Game game;
    if (bool success = game.Initialize()) {
        game.RunLoop();
    }
    game.Shutdown();

    networkingCleanup();
    return 0;
}