#include "ui/GameMenu.h"
#include "ui/Colors.h"
#include "core/Game.h"

#include <string>

GameMenu::GameMenu(Game& game)
    : game_(game),
      renderer_(game.renderer()),
      panel_(game.assets().texture("otherassets/gameMenu.png")),
      score_(renderer_, game.assets().font(25)),
      partyView_(game.assets(), PartyView::Style::Menu),
      highScorePanel_(game.assets().texture("battleassets/pokemon_sel_screen.png")),
      highScoreBack_(Button::single(game.assets().texture("otherassets/backButton.png"), 592, 473)) {
    Assets& assets = game.assets();
    SDL_QueryTexture(panel_, nullptr, nullptr, &panelRect_.w, &panelRect_.h);
    buttons_[POKEMON] = Button::single(assets.texture("otherassets/pokemonButton.png"), 559, 87);
    buttons_[SAVE] = Button::single(assets.texture("otherassets/saveButton.png"), 559, 182);
    buttons_[HIGH_SCORES] = Button::single(assets.texture("otherassets/highscoreButton.png"), 559, 277);
    buttons_[CLOSE] = Button::single(assets.texture("otherassets/quitButton.png"), 559, 372);
    for (TextLabel& text : highScoreTexts_) text = TextLabel(renderer_, assets.font(28));
}

void GameMenu::handleEvent(const SDL_Event& e) {
    const Sounds& sounds = game_.sounds();
    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE && !inPokemonView_ && !inHighScoreView_) {
        open_ = false;
        return;
    }

    if (!inPokemonView_ && !inHighScoreView_) {
        for (Button& button : buttons_) button.handleEvent(e);

        if (buttons_[POKEMON].clicked()) {
            buttons_[POKEMON].resetClick();
            if (!game_.player().hasPokemon()) {
                playSound(sounds.denied);
            } else {
                playSound(sounds.clickedOn);
                partyView_.update(game_.player().party);
                inPokemonView_ = true;
            }
            return;
        }
        if (buttons_[SAVE].clicked()) {
            buttons_[SAVE].resetClick();
            playSound(sounds.gameSaved);
            game_.savePlayer();
            open_ = false;
            return;
        }
        if (buttons_[HIGH_SCORES].clicked()) {
            buttons_[HIGH_SCORES].resetClick();
            playSound(sounds.clickedOn);
            inHighScoreView_ = true;
            return;
        }
        if (buttons_[CLOSE].clicked()) {
            buttons_[CLOSE].resetClick();
            playSound(sounds.clickedOn);
            open_ = false;
            return;
        }
    }

    if (inPokemonView_) {
        Button& back = partyView_.backButton();
        back.handleEvent(e);
        if (back.clicked()) {
            back.resetClick();
            playSound(sounds.clickedOn);
            inPokemonView_ = false;
            return;
        }
    }

    if (inHighScoreView_) {
        highScoreBack_.handleEvent(e);
        if (highScoreBack_.clicked()) {
            highScoreBack_.resetClick();
            playSound(sounds.clickedOn);
            inHighScoreView_ = false;
            return;
        }
    }
}

void GameMenu::draw() {
    SDL_RenderCopy(renderer_, panel_, nullptr, &panelRect_);
    for (const Button& button : buttons_) button.draw(renderer_);

    score_.setText("Score: " + std::to_string(game_.player().currentScore()), TEXT_BLACK);
    score_.draw(568, 479);

    if (inPokemonView_) partyView_.draw(renderer_);
    if (inHighScoreView_) drawHighScores();
}

void GameMenu::drawHighScores() {
    const SDL_Rect panel = {91, 152, 650, 400};
    SDL_RenderCopy(renderer_, highScorePanel_, nullptr, &panel);

    const auto& scores = game_.player().highScores.scores();
    for (int i = 0; i < 5; i++) {
        highScoreTexts_[i].setText(std::to_string(i + 1) + ". " + std::to_string(scores[i]), TEXT_BLACK);
        highScoreTexts_[i].draw(155, 217 + 53 * i);
    }
    highScoreBack_.draw(renderer_);
}
