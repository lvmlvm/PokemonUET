#include "world/Npc.h"
#include "world/Direction.h"

namespace {
const int SPRITE_W = 64, SPRITE_H = 88;
} // namespace

Npc::Npc(int x, int y, int facing, SDL_Texture* spriteSheet, bool isTrainer)
    : x_(x), y_(y), facing_(facing), spriteSheet_(spriteSheet), isTrainer_(isTrainer) {}

const std::vector<std::string>& Npc::activeDialogue() const {
    return (isTrainer_ && !hasBattled_) ? preBattleDialogue_ : dialogue_;
}

Npc::Talk Npc::talk(int playerFacing) {
    if (playerFacing >= SOUTH && playerFacing <= WEST) facing_ = oppositeDirection(playerFacing);

    if (cursor_ < activeDialogue().size()) {
        cursor_++;
        return Talk::Talking;
    }
    cursor_ = 0;
    return (isTrainer_ && !hasBattled_) ? Talk::StartBattle : Talk::Finished;
}

std::string Npc::currentSentence() const {
    const std::vector<std::string>& sentences = activeDialogue();
    if (cursor_ == 0 || cursor_ > sentences.size()) return "";
    return sentences[cursor_ - 1];
}

void Npc::draw(SDL_Renderer* renderer, int camX, int camY) const {
    // Standing frame for each direction: frames 0, 4, 8, 12 of the walk sheet.
    int frame = (facing_ >= SOUTH && facing_ <= WEST) ? facing_ * 4 : 0;
    SDL_Rect src = {frame * SPRITE_W, 0, SPRITE_W, SPRITE_H};
    SDL_Rect dest = {x_ * 64 - camX, y_ * 64 - camY - 24, SPRITE_W, SPRITE_H};
    SDL_RenderCopy(renderer, spriteSheet_, &src, &dest);
}
