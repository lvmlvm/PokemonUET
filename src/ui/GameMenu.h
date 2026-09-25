#pragma once

#include "ui/Button.h"
#include "ui/PartyView.h"
#include "ui/TextLabel.h"

#include <array>

class Game;

// The overworld menu (V): party, save, high scores, close.
class GameMenu {
public:
    explicit GameMenu(Game& game);

    bool isOpen() const { return open_; }
    void open() { open_ = true; }
    void handleEvent(const SDL_Event& event);
    void draw();

private:
    void drawHighScores();

    Game& game_;
    SDL_Renderer* renderer_;
    SDL_Texture* panel_;
    SDL_Rect panelRect_ = {538, 43, 0, 0};
    enum { POKEMON, SAVE, HIGH_SCORES, CLOSE };
    std::array<Button, 4> buttons_;
    TextLabel score_;
    PartyView partyView_;
    SDL_Texture* highScorePanel_;
    std::array<TextLabel, 5> highScoreTexts_;
    Button highScoreBack_;
    bool open_ = false, inPokemonView_ = false, inHighScoreView_ = false;
};
