#include "iostream"
#include "core/Music.h"

Music::Music() {
    gameTheme = NULL;
    pastChord = false;
}

Music::~Music() {
    freeMusic();
    pastChord = false;
}

void Music::freeMusic() {
    Mix_FreeMusic(gameTheme);
    gameTheme=NULL;
}

void Music::loadMusic(const char* path, double repeatP) {
    freeMusic();
    gameTheme = Mix_LoadMUS(path);
    repeatPoint = repeatP;
}

void Music::play() {
    if (Mix_PlayingMusic() == 0) {
        Mix_PlayMusic(gameTheme, 1);
        if (pastChord == true) Mix_SetMusicPosition(repeatPoint);
        else pastChord = true;
    }
}

void Music::manualSkip(double test) {
    Mix_SetMusicPosition(test);
    std::cout << "Manual skipped!\n";
}