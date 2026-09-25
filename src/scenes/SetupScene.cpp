#include "scenes/SetupScene.h"
#include "ui/Colors.h"
#include "scenes/OverworldScene.h"
#include "core/Replay.h"

namespace {
const int FADE_STEP = 17;
} // namespace

SetupScene::SetupScene(Game& game)
    : game_(game),
      renderer_(game.renderer()),
      prompt_(renderer_, game.assets().font(40)),
      ruby_(Button::horizontal(game.assets().texture("titlescreen/rubybutton.png"), {111, 157, 241, 444}, 241, 444)),
      sapphire_(Button::horizontal(game.assets().texture("titlescreen/sapphirebutton.png"), {480, 157, 241, 444}, 241, 444)) {
    prompt_.setText("Choose your character:", TEXT_WHITE);
}

void SetupScene::handleEvent(const SDL_Event& e) {
    const int genders[2] = {0, 1};
    Button* choices[2] = {&ruby_, &sapphire_};
    for (int i = 0; i < 2; i++) {
        choices[i]->handleEvent(e);
        if (choices[i]->clicked() && transition_ != Transition::ToOverworld) {
            choices[i]->resetClick();
            playSound(game_.sounds().clickedOn);
            game_.player().setGender(genders[i]);
            transition_ = Transition::ToOverworld;
            return;
        }
    }
}

void SetupScene::frame() {
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);

    prompt_.draw(196, 58);
    ruby_.draw(renderer_);
    sapphire_.draw(renderer_);

    Fade& fade = game_.fade();
    if (transition_ == Transition::FadeIn) {
        if (fade.in(FADE_STEP)) transition_ = Transition::None;
        fade.draw(renderer_);
    }
    if (transition_ == Transition::ToOverworld) {
        if (fade.out(FADE_STEP)) {
            game_.replaceScene(std::make_unique<OverworldScene>(game_));
            replay::delay(1000);
        }
        fade.draw(renderer_);
    }
}
