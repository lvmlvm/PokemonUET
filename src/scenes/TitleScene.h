#pragma once

#include "core/Game.h"
#include "ui/Button.h"

#include <array>

// Splash screen, animated logo, and Continue / New Game / Help / Quit.
class TitleScene : public Scene {
public:
    explicit TitleScene(Game& game);

    void handleEvent(const SDL_Event& event) override;
    void frame() override;

private:
    enum class Transition { None, ToOverworld, ToSetup };
    enum { CONTINUE, NEW_GAME, HELP, QUIT };

    void draw();
    void animateButton(int button, int& animPos, int& transparency, bool startNow, int y);

    Game& game_;
    SDL_Renderer* renderer_;
    bool hasSaveFile_;
    TexturePtr splash_, background_, logo_, helpScreen_;
    std::array<TexturePtr, 4> buttonTextures_; // own copies: faded in via alpha
    std::array<Button, 4> buttons_;            // CONTINUE only exists with a save file
    TexturePtr backTexture_;
    Button backButton_;
    Transition transition_ = Transition::None;

    // Intro animation
    int splashTransparency_ = 254;
    int logoTransparency_ = 1;
    int logoY_ = 175;
    SDL_Rect logoRect_;
    std::array<int, 4> buttonAnimPos_ = {60, 60, 60, 60};
    std::array<int, 4> buttonTransparency_ = {0, 0, 0, 0};
    bool acceptInput_ = false;
    bool inHelpScreen_ = false;
};
