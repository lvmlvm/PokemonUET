#include "core/Music.h"

void Music::load(const std::string& path, double repeatPoint) {
    theme_.reset(); // stops the current theme
    theme_.reset(Mix_LoadMUS(path.c_str()));
    if (!theme_) SDL_Log("Failed to load music %s: %s", path.c_str(), Mix_GetError());
    repeatPoint_ = repeatPoint;
}

void Music::play() {
    if (!theme_ || Mix_PlayingMusic() != 0) return;
    Mix_PlayMusic(theme_.get(), 1);
    if (pastChord_) Mix_SetMusicPosition(repeatPoint_);
    else pastChord_ = true;
}

void Music::stop() {
    Mix_HaltMusic();
    theme_.reset();
}
