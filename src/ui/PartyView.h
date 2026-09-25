#pragma once

#include "core/Assets.h"
#include "battle/Pokemon.h"
#include "ui/Button.h"
#include "ui/TextLabel.h"

#include <array>

// The party screen: each Pokemon's name, HP bar and whether it can still battle. In battle it
// also asks which Pokemon to send out, and each row is clickable.
class PartyView {
public:
    enum class Style { Menu, Battle };

    PartyView(Assets& assets, Style style);

    void update(const Pokemon party[3]);
    void draw(SDL_Renderer* renderer);

    Button& backButton() { return backButton_; }
    Button& slotButton(int slot) { return slotButtons_[slot]; } // Battle style only

private:
    SDL_Renderer* renderer_;
    Style style_;
    SDL_Texture* background_;
    SDL_Texture* canBattle_;
    SDL_Texture* cannotBattle_;
    SDL_Texture* hpBar_;
    SDL_Texture* hpColor_;
    std::array<SDL_Rect, 3> hpFill_ = {};
    std::array<TextLabel, 3> names_;
    std::array<TextLabel, 3> hpTexts_;
    TextLabel prompt_;
    Button backButton_;
    std::array<Button, 3> slotButtons_;
};
