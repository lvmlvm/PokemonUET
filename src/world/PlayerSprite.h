#pragma once

#include "core/Sdl.h"

// Draws the player at the center of the screen: standing, walking or running.
class PlayerSprite {
public:
    PlayerSprite() = default;
    explicit PlayerSprite(SDL_Texture* spriteSheet) : sheet_(spriteSheet) {}

    void drawStanding(SDL_Renderer* renderer, int facing) const;
    void drawWalking(SDL_Renderer* renderer, int facing);
    void drawRunning(SDL_Renderer* renderer, int facing);

private:
    void drawFrame(SDL_Renderer* renderer, int frame) const;
    void advance(int framesPerStep);

    SDL_Texture* sheet_ = nullptr;
    int moveFrame_ = 0; // counts rendered frames while moving
    int walkFrame_ = 0; // 0..3, the step of the walk cycle
};
