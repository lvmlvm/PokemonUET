#include "ui/PartyView.h"
#include "ui/Colors.h"

#include <cmath>
#include <string>

namespace {
const SDL_Rect PANEL = {91, 152, 650, 400};
const int ROW_Y = 244, ROW_HEIGHT = 78;
} // namespace

PartyView::PartyView(Assets& assets, Style style)
    : renderer_(assets.renderer()),
      style_(style),
      background_(assets.texture("battleassets/pokemon_sel_screen.png")),
      canBattle_(assets.texture("battleassets/canBattleIcon.png")),
      cannotBattle_(assets.texture("battleassets/cannotBattleIcon.png")),
      hpBar_(assets.texture("battleassets/inPartyHP.png")),
      hpColor_(assets.texture("battleassets/HPColor.png")),
      prompt_(renderer_, assets.font(30)) {
    for (int i = 0; i < 3; i++) {
        names_[i] = TextLabel(renderer_, assets.font(42));
        hpTexts_[i] = TextLabel(renderer_, assets.font(28));
    }
    if (style == Style::Menu) {
        backButton_ = Button::single(assets.texture("otherassets/backButton.png"), 592, 473);
    } else {
        backButton_ = Button::vertical(assets.texture("battleassets/backbutton.png"), {592, 473, 113, 43}, 362, 139);
        prompt_.setText("Which Pokemon to sub in?", TEXT_BLACK);
        for (int i = 0; i < 3; i++) slotButtons_[i] = Button::vertical(nullptr, {123, 242 + ROW_HEIGHT * i, 547, 58}, 0, 0);
    }
}

void PartyView::update(const Pokemon party[3]) {
    for (int i = 0; i < 3; i++) {
        const Pokemon& pokemon = party[i];
        hpFill_[i] = {541, 284 + ROW_HEIGHT * i, int(std::ceil(121.0 * (double(pokemon.c_hp) / double(pokemon.data->hp)))), 8};
        names_[i].setText(pokemon.data->name, TEXT_BLACK);
        hpTexts_[i].setText(std::to_string(pokemon.c_hp) + "/" + std::to_string(pokemon.data->hp), TEXT_BLACK);
    }
}

void PartyView::draw(SDL_Renderer* renderer) {
    SDL_RenderCopy(renderer, background_, nullptr, &PANEL);
    if (style_ == Style::Battle) prompt_.draw(123, 184);

    for (int i = 0; i < 3; i++) names_[i].draw(173, ROW_Y + ROW_HEIGHT * i);

    for (int i = 0; i < 3; i++) {
        SDL_Rect ball = {124, ROW_Y + ROW_HEIGHT * i, 36, 36};
        SDL_Rect bar = {500, 280 + ROW_HEIGHT * i, 168, 18};
        SDL_RenderCopy(renderer, hpFill_[i].w == 0 ? cannotBattle_ : canBattle_, nullptr, &ball);
        SDL_RenderCopy(renderer, hpBar_, nullptr, &bar);
        SDL_RenderCopy(renderer, hpColor_, nullptr, &hpFill_[i]);
    }

    for (int i = 0; i < 3; i++) hpTexts_[i].draw(500, ROW_Y + ROW_HEIGHT * i);

    backButton_.draw(renderer);
}
