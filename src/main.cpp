#include "core/Game.h"

#include <exception>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    try {
        Game game;
        game.run();
    } catch (const std::exception& error) {
        SDL_Log("Fatal error: %s", error.what());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Pokémon UET", error.what(), nullptr);
        return 1;
    }
    return 0;
}
