#pragma once

#include "core/Game.h"
#include "ui/Button.h"
#include "ui/TextLabel.h"

// New game: choose between Ruby and Sapphire.
class SetupScene : public Scene {
public:
    explicit SetupScene(Game& game);

    void handleEvent(const SDL_Event& event) override;
    void frame() override;

private:
    enum class Transition { FadeIn, None, ToOverworld };

    Game& game_;
    SDL_Renderer* renderer_;
    TextLabel prompt_;
    Button ruby_, sapphire_;
    Transition transition_ = Transition::FadeIn;
};
