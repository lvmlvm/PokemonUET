#include "world/PlayerSprite.h"
#include "world/Direction.h"

namespace {
const int SPRITE_W = 64, SPRITE_H = 88;
const SDL_Rect SCREEN_CENTER = {(832 - 64) / 2, (704 - 64) / 2 - 24, SPRITE_W, SPRITE_H};

bool validFacing(int facing) {
    return facing >= SOUTH && facing <= WEST;
}
} // namespace

// Sheet layout: 4 walking frames per direction (frames 0-15), then 4 running frames per
// direction (frames 16-31), in SOUTH, EAST, NORTH, WEST order.
void PlayerSprite::drawFrame(SDL_Renderer* renderer, int frame) const {
    SDL_Rect src = {frame * SPRITE_W, 0, SPRITE_W, SPRITE_H};
    SDL_RenderCopy(renderer, sheet_, &src, &SCREEN_CENTER);
}

void PlayerSprite::advance(int framesPerStep) {
    moveFrame_++;
    if (moveFrame_ > 60) moveFrame_ = 1;
    if (moveFrame_ % framesPerStep == 0) walkFrame_++;
    if (walkFrame_ > 3) walkFrame_ = 0;
}

void PlayerSprite::drawStanding(SDL_Renderer* renderer, int facing) const {
    drawFrame(renderer, validFacing(facing) ? facing * 4 : 0);
}

void PlayerSprite::drawWalking(SDL_Renderer* renderer, int facing) {
    advance(10);
    drawFrame(renderer, validFacing(facing) ? facing * 4 + walkFrame_ : 0);
}

void PlayerSprite::drawRunning(SDL_Renderer* renderer, int facing) {
    advance(5);
    drawFrame(renderer, validFacing(facing) ? 16 + facing * 4 + walkFrame_ : 0);
}
