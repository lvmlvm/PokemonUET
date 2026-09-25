#pragma once

#include <SDL.h>

// Text colors. The alpha of 0 is what the game has always passed to SDL_ttf; it is kept so
// text renders exactly as before.
const SDL_Color TEXT_BLACK = {0, 0, 0, 0};
const SDL_Color TEXT_WHITE = {255, 255, 255, 0};
