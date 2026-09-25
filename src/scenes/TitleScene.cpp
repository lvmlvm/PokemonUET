#include "scenes/TitleScene.h"
#include "scenes/OverworldScene.h"
#include "scenes/SetupScene.h"

namespace {
const int BUTTON_W = 210, BUTTON_H = 55;
const int BUTTON_X = (Game::SCREEN_WIDTH - BUTTON_W) / 2;
const int FADE_STEP = 17;
const char* BUTTON_IMAGES[4] = {"titlescreen/button_continue.png", "titlescreen/button_newgame.png",
                                "titlescreen/button_help.png", "titlescreen/button_quit.png"};
} // namespace

TitleScene::TitleScene(Game& game)
    : game_(game),
      renderer_(game.renderer()),
      hasSaveFile_(game.hasSaveFile()),
      logoRect_({(Game::SCREEN_WIDTH - 500) / 2, logoY_, 500, 284}) {
    Assets& assets = game.assets();
    splash_ = assets.loadTexture("titlescreen/splash.png");
    background_ = assets.loadTexture("titlescreen/background.png");
    logo_ = assets.loadTexture("titlescreen/logo.png");
    helpScreen_ = assets.loadTexture("titlescreen/helpScreen.png");
    for (SDL_Texture* texture : {splash_.get(), background_.get(), logo_.get()}) SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    for (int i = hasSaveFile_ ? CONTINUE : NEW_GAME; i <= QUIT; i++) {
        buttonTextures_[i] = assets.loadTexture(BUTTON_IMAGES[i]);
        SDL_SetTextureBlendMode(buttonTextures_[i].get(), SDL_BLENDMODE_BLEND);
        buttons_[i] = Button::vertical(buttonTextures_[i].get(), {0, 0, BUTTON_W, BUTTON_H}, BUTTON_W, BUTTON_H);
    }
    backTexture_ = assets.loadTexture("titlescreen/back_button.png");
    backButton_ = Button::horizontal(backTexture_.get(), {18, 636, 147, 56}, 362, 139);
}

// INPUT

void TitleScene::handleEvent(const SDL_Event& e) {
    if (!acceptInput_) return;
    const Sounds& sounds = game_.sounds();

    for (int i = hasSaveFile_ ? CONTINUE : NEW_GAME; i <= QUIT; i++) buttons_[i].handleEvent(e);

    if (inHelpScreen_) {
        backButton_.handleEvent(e);
        if (backButton_.clicked()) {
            backButton_.resetClick();
            playSound(sounds.clickedOn);
            inHelpScreen_ = false;
        }
    }

    if (hasSaveFile_ && buttons_[CONTINUE].clicked()) {
        playSound(sounds.clickedOn);
        buttons_[CONTINUE].resetClick();
        acceptInput_ = false;
        transition_ = Transition::ToOverworld;
    } else if (buttons_[NEW_GAME].clicked()) {
        playSound(sounds.clickedOn);
        buttons_[NEW_GAME].resetClick();
        game_.player().reset();
        acceptInput_ = false;
        transition_ = Transition::ToSetup;
    } else if (buttons_[HELP].clicked()) {
        playSound(sounds.clickedOn);
        buttons_[HELP].resetClick();
        inHelpScreen_ = true;
    } else if (buttons_[QUIT].clicked()) {
        playSound(sounds.clickedOn);
        game_.quit();
    }
}

// FRAME

void TitleScene::frame() {
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);

    draw();

    Fade& fade = game_.fade();
    if (transition_ == Transition::ToOverworld) {
        if (fade.out(FADE_STEP) && ++blackFrames_ == 60) { // a second of black
            game_.replaceScene(std::make_unique<OverworldScene>(game_));
        }
        fade.draw(renderer_);
    }
    if (transition_ == Transition::ToSetup) {
        if (fade.out(FADE_STEP) && ++blackFrames_ == 30) { // half a second of black
            game_.replaceScene(std::make_unique<SetupScene>(game_));
        }
        fade.draw(renderer_);
    }
}

// Slides a button up 4px per frame (from 60px below its place) while fading it in.
void TitleScene::animateButton(int button, int& animPos, int& transparency, bool startNow, int y) {
    if (startNow && animPos > 0) {
        animPos -= 4;
        if (transparency < 255) transparency += 17;
    }
    buttons_[button].setPosition(BUTTON_X, y + animPos);
    SDL_SetTextureAlphaMod(buttons_[button].texture(), transparency);
}

void TitleScene::draw() {
    // The splash screen holds for 3 seconds, then fades out over the campus background.
    if (splashTransparency_ < 254) SDL_RenderCopy(renderer_, background_.get(), nullptr, nullptr);
    SDL_SetTextureAlphaMod(splash_.get(), splashTransparency_);
    SDL_RenderCopy(renderer_, splash_.get(), nullptr, nullptr);
    if (splashTransparency_ == 252 && splashHoldFrames_ < 180) splashHoldFrames_++;
    else if (splashTransparency_ > 0) splashTransparency_ -= 2;

    // Then the logo fades in while rising to y = 50.
    if (splashTransparency_ == 0) {
        SDL_SetTextureAlphaMod(logo_.get(), logoTransparency_);
        SDL_RenderCopy(renderer_, logo_.get(), nullptr, &logoRect_);
        if (logoTransparency_ < 255) logoTransparency_ += 2;
        if (logoY_ > 50) {
            logoY_--;
            logoRect_.y = logoY_;
        }
    }

    // Then the buttons slide in one after the other.
    auto& pos = buttonAnimPos_;
    auto& alpha = buttonTransparency_;
    if (logoY_ <= 50) {
        const int bottom = Game::SCREEN_HEIGHT;
        if (hasSaveFile_) {
            animateButton(CONTINUE, pos[0], alpha[0], true, bottom - 65 * 5);
            animateButton(NEW_GAME, pos[1], alpha[1], pos[0] < 30, bottom - 65 * 4);
            animateButton(HELP, pos[2], alpha[2], pos[1] < 30, bottom - 65 * 3);
            animateButton(QUIT, pos[3], alpha[3], pos[2] < 30, bottom - 65 * 2);
            for (int i = CONTINUE; i <= QUIT; i++) buttons_[i].draw(renderer_);
        } else {
            animateButton(NEW_GAME, pos[1], alpha[1], true, bottom - 65 * 5 + 28);
            animateButton(HELP, pos[2], alpha[2], pos[1] < 30, bottom - 65 * 4 + 28);
            animateButton(QUIT, pos[3], alpha[3], pos[2] < 30, bottom - 65 * 3 + 28);
            for (int i = NEW_GAME; i <= QUIT; i++) buttons_[i].draw(renderer_);
        }
    }
    if (pos[QUIT] == 0 && transition_ == Transition::None) acceptInput_ = true;

    if (inHelpScreen_) {
        SDL_RenderCopy(renderer_, helpScreen_.get(), nullptr, nullptr);
        backButton_.draw(renderer_);
    }
}
