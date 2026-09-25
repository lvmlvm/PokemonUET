#include "core/SdlContext.h"

#include <stdexcept>
#include <string>

SdlContext::Libraries::Libraries() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0) {
        throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());
    }
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
        SDL_Log("Mix_OpenAudio failed, continuing without sound: %s", Mix_GetError());
    }
}

SdlContext::Libraries::~Libraries() {
    Mix_CloseAudio();
    TTF_Quit();
    Mix_Quit();
    IMG_Quit();
    SDL_Quit();
}

SdlContext::SdlContext(const char* title, int width, int height) {
    window_.reset(SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN));
    if (!window_) {
        throw std::runtime_error(std::string("SDL_CreateWindow failed: ") + SDL_GetError());
    }
    renderer_.reset(SDL_CreateRenderer(window_.get(), -1, SDL_RENDERER_ACCELERATED));
    if (!renderer_) { // e.g. no GPU available: fall back to the software renderer
        renderer_.reset(SDL_CreateRenderer(window_.get(), -1, SDL_RENDERER_SOFTWARE));
    }
    if (!renderer_) {
        throw std::runtime_error(std::string("SDL_CreateRenderer failed: ") + SDL_GetError());
    }
}
