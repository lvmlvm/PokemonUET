#include "scenes/TitleScreen.h"
#include "core/RenderWindow.h"
#include "Globals.h"
#include "core/Replay.h"
#include <iostream>

// TITLE SCREEN:

static int logoYCoord = 175;
static int splashScreenTransparency = 254;
static int logoTransparency = 1;
static SDL_Rect logoDestBox = {(832-500)/2, logoYCoord, 500, 284};

const int tsBW = 210;
const int tsBH = 55;

static int buttonAnimPos0 = 60;
static int buttonAnimPos1 = 60;
static int buttonAnimPos2 = 60;
static int buttonAnimPos3 = 60;
static int buttonTransparency0 = 0;
static int buttonTransparency1 = 0;
static int buttonTransparency2 = 0;
static int buttonTransparency3 = 0;

TitleScreen::TitleScreen() {
    splashScreenTexture = NULL;
    titleScreenBackground = NULL;
    titleScreenLogo = NULL;
    helpScreenTexture = NULL;
    presentSaveFile = false;
    inHelpScreen = false;
    acceptInput = false;
}

TitleScreen::~TitleScreen() {
    freeTitleScreen();
}

void TitleScreen::freeTitleScreen() {
    SDL_Texture** textures[] = {&splashScreenTexture, &titleScreenBackground, &titleScreenLogo, &helpScreenTexture};
    for (SDL_Texture** texture : textures) {
        if (*texture != NULL) SDL_DestroyTexture(*texture);
        *texture = NULL;
    }
    logoYCoord = 175;
    for (int i = 0; i < 4; i++) tsButtons[i].freeButton();
    backButton.freeButton();
}

void TitleScreen::initTitleScreen(bool hasSaveFile) {
    splashScreenTexture = IMG_LoadTexture(RenderWindow::renderer, "res/titlescreen/splash.png");
    titleScreenBackground = IMG_LoadTexture(RenderWindow::renderer, "res/titlescreen/background.png");
    titleScreenLogo = IMG_LoadTexture(RenderWindow::renderer, "res/titlescreen/logo.png");
    helpScreenTexture = IMG_LoadTexture(RenderWindow::renderer, "res/titlescreen/helpScreen.png");
    presentSaveFile = hasSaveFile;

    SDL_SetTextureBlendMode(splashScreenTexture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(titleScreenBackground, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(titleScreenLogo, SDL_BLENDMODE_BLEND);
}

void TitleScreen::drawTitleScreen() {
    // Renders the VNU Background at the back
    if (splashScreenTransparency < 254) SDL_RenderCopy(RenderWindow::renderer, titleScreenBackground, NULL, NULL);

    // Handles the splash screen
    SDL_SetTextureAlphaMod(splashScreenTexture, splashScreenTransparency);
    SDL_RenderCopy(RenderWindow::renderer, splashScreenTexture, NULL, NULL);

    // Fades the splash screen away
    if (splashScreenTransparency == 252) replay::delay(3000);
    if (splashScreenTransparency > 0) splashScreenTransparency -= 2;

    // Animate the Pokemon VNU logo
    if (splashScreenTransparency == 0) {
        SDL_SetTextureAlphaMod(titleScreenLogo, logoTransparency);
        SDL_RenderCopy(RenderWindow::renderer, titleScreenLogo, NULL, &logoDestBox);
        if (logoTransparency < 255) {
            logoTransparency += 2;
        }
        if (logoYCoord > 50) {
            logoYCoord--;
            logoDestBox = {(832-500)/2, logoYCoord, 500, 284};
        }
    }

    // Animate the 4 buttons
    if (logoYCoord <= 50) {
        if (presentSaveFile == true) {
            // ANIMATING CONTINUE BUTTON
            if (buttonAnimPos0 > 0) {
                buttonAnimPos0 -= 4;
                if (buttonTransparency0 < 255) buttonTransparency0 += 17;
            }
            tsButtons[0].setButtonDest((832 - 210)/2, 704 - 65 * 5 + buttonAnimPos0);
            SDL_SetTextureAlphaMod(tsButtons[0].getButtonTexture(), buttonTransparency0);
            
            // ANIMATING NEW GAME BUTTON
            if (buttonAnimPos0 < 30 and buttonAnimPos1 > 0) {
                buttonAnimPos1 -= 4;
                if (buttonTransparency1 < 255) buttonTransparency1 += 17;
            }
            tsButtons[1].setButtonDest((832 - 210)/2, 704 - 65 * 4 + buttonAnimPos1);
            SDL_SetTextureAlphaMod(tsButtons[1].getButtonTexture(), buttonTransparency1);

            // ANIMATING HELP BUTTON
            if (buttonAnimPos1 < 30 and buttonAnimPos2 > 0) {
                buttonAnimPos2 -= 4;
                if (buttonTransparency2 < 255) buttonTransparency2 += 17;
            }
            tsButtons[2].setButtonDest((832 - 210)/2, 704 - 65 * 3 + buttonAnimPos2);
            SDL_SetTextureAlphaMod(tsButtons[2].getButtonTexture(), buttonTransparency2);

            // ANIMATING QUIT BUTTON
            if (buttonAnimPos2 < 30 and buttonAnimPos3 > 0) {
                buttonAnimPos3 -= 4;
                if (buttonTransparency3 < 255) buttonTransparency3 += 17;
            }
            tsButtons[3].setButtonDest((832 - 210)/2, 704 - 65 * 2 + buttonAnimPos3);
            SDL_SetTextureAlphaMod(tsButtons[3].getButtonTexture(), buttonTransparency3);

            // GET THE BUTTONS ON THE RENDERING BUFFER
            for (int i = 0; i < 4; i++) {
                tsButtons[i].drawButton();
            }
        } else {
            // ANIMATING NEW GAME BUTTON
            if (buttonAnimPos1 > 0) {
                buttonAnimPos1 -= 4;
                if (buttonTransparency1 < 255) buttonTransparency1 += 17;
            }
            tsButtons[1].setButtonDest((832 - 210)/2, 704 - 65 * 5 + buttonAnimPos1 + 28);
            SDL_SetTextureAlphaMod(tsButtons[1].getButtonTexture(), buttonTransparency1);

            // ANIMATING HELP BUTTON
            if (buttonAnimPos1 < 30 and buttonAnimPos2 > 0) {
                buttonAnimPos2 -= 4;
                if (buttonTransparency2 < 255) buttonTransparency2 += 17;
            }
            tsButtons[2].setButtonDest((832 - 210)/2, 704 - 65 * 4 + buttonAnimPos2 + 28);
            SDL_SetTextureAlphaMod(tsButtons[2].getButtonTexture(), buttonTransparency2);

            // ANIMATING QUIT BUTTON
            if (buttonAnimPos2 < 30 and buttonAnimPos3 > 0) {
                buttonAnimPos3 -= 4;
                if (buttonTransparency3 < 255) buttonTransparency3 += 17;
            }
            tsButtons[3].setButtonDest((832 - 210)/2, 704 - 65 * 3 + buttonAnimPos3 + 28);
            SDL_SetTextureAlphaMod(tsButtons[3].getButtonTexture(), buttonTransparency3);

            for (int i = 1; i < 4; i++) {
                tsButtons[i].drawButton();
            }
        }
    }
    if (buttonAnimPos3 == 0) acceptInput = true;

    if (inHelpScreen == true) {
        SDL_RenderCopy(RenderWindow::renderer, helpScreenTexture, NULL, NULL);
        backButton.drawButton();
    }
}

void TitleScreen::tsButtonInit() {
    // 0: CONTINUE, 1: NEW GAME, 2: HELP, 3: QUIT
    if (presentSaveFile == true) {
        tsButtons[0].initTSBTexture("res/titlescreen/button_continue.png");
        tsButtons[1].initTSBTexture("res/titlescreen/button_newgame.png");
        tsButtons[2].initTSBTexture("res/titlescreen/button_help.png");
        tsButtons[3].initTSBTexture("res/titlescreen/button_quit.png");
    } else {
        tsButtons[1].initTSBTexture("res/titlescreen/button_newgame.png");
        tsButtons[2].initTSBTexture("res/titlescreen/button_help.png");
        tsButtons[3].initTSBTexture("res/titlescreen/button_quit.png");
    }

    backButton.initSSB("res/titlescreen/back_button.png", 18, 636, 147, 56, 362, 139);
}

bool TitleScreen::acceptInputState() {
    return acceptInput;
}

void TitleScreen::stopInputState() {
    acceptInput = false;
}

void TitleScreen::doButtonEvents(SDL_Event* e) {
    if (presentSaveFile == true) {
        for (int i = 0; i < 4; i++) {
            tsButtons[i].buttonHandler(e);
        }
    } else {
        for (int i = 1; i < 4; i++) {
            tsButtons[i].buttonHandler(e);
        }
    }

    if (inHelpScreen == true) {
        backButton.buttonHandler(e);
        if (backButton.clickedOn == true) {
            backButton.clickedOn = false;
            Mix_PlayChannel(-1, clickedOnSound, 0);
            inHelpScreen = false;
            return;
        }
    }
}

// TITLE SCREEN BUTTON:

TitleScreenButton::TitleScreenButton() {
    buttonTexture = NULL;
    clickedOn = false;
    currentButtonFrame = 0;
    buttonDest = {0, 0, 0, 0};
}

TitleScreenButton::~TitleScreenButton() {
    freeButton();
}

void TitleScreenButton::freeButton() {
    if (buttonTexture != NULL) SDL_DestroyTexture(buttonTexture);
    clickedOn = false;
    buttonTexture = NULL;
}

void TitleScreenButton::initTSBTexture(const char* path) {
    freeButton();
    buttonTexture = IMG_LoadTexture(RenderWindow::renderer, path);
    SDL_SetTextureBlendMode(buttonTexture, SDL_BLENDMODE_BLEND);
    for (int i = 0; i < 3; i++) {
        buttonFrames[i] = {0, tsBH*i, tsBW, tsBH};
    }
}

void TitleScreenButton::setButtonDest(int x, int y) {
    buttonDest = {x, y, tsBW, tsBH};
}

void TitleScreenButton::drawButton() {
    SDL_RenderCopy(RenderWindow::renderer, buttonTexture, &buttonFrames[currentButtonFrame], &buttonDest);
}

void TitleScreenButton::buttonHandler(SDL_Event* e) {
    // 0: INSIDE, 1: HOVER, 2: CLICK
    if (e->type == SDL_MOUSEMOTION or e->type == SDL_MOUSEBUTTONDOWN or e->type == SDL_MOUSEBUTTONUP) {
        int x, y;
        eventMousePosition(*e, x, y);

        bool inside = true;
        if (x < buttonDest.x) inside = false;
        else if (x > buttonDest.x + tsBW) inside = false;
        else if (y < buttonDest.y) inside = false;
        else if (y > buttonDest.y + tsBH) inside = false;

        if (inside == false) {
            currentButtonFrame = 0;
        } else {
            switch (e->type) {
                case SDL_MOUSEMOTION:
                currentButtonFrame = 1;
                break;

                case SDL_MOUSEBUTTONDOWN:
                currentButtonFrame = 2;
                break;

                case SDL_MOUSEBUTTONUP:
                currentButtonFrame = 1;
                clickedOn = true;
                break;

                default:
                break;
            }
        }
    }
}

bool TitleScreenButton::isClicked() {
    return clickedOn;
}

void TitleScreenButton::resetClickState() {
    clickedOn = false;
}

SDL_Texture* TitleScreenButton::getButtonTexture() {
    return buttonTexture;
}

// SETUP SCREEN

SetupScreen::SetupScreen() {}
SetupScreen::~SetupScreen() {}

void SetupScreen::initSetupScreen() {
    chooseUrGender.createFont("res/font/gamefont.ttf", 40);
    rubyChoice.initSSB("res/titlescreen/rubybutton.png", 111, 157, 241, 444, 241, 444);
    sapphireChoice.initSSB("res/titlescreen/sapphirebutton.png", 480, 157, 241, 444, 241, 444);
}

void SetupScreen::freeSetupScreen() {
    chooseUrGender.freeText();
    rubyChoice.freeButton();
    sapphireChoice.freeButton();
}

void SetupScreen::drawSetupScreen() {
    chooseUrGender.textInit(RenderWindow::renderer, "Choose your character:", {255,255,255});
    chooseUrGender.display(196, 58, RenderWindow::renderer);

    rubyChoice.drawButton();
    sapphireChoice.drawButton();
}

void SetupScreen::setupScreenInputProcess(SDL_Event* e) {
    if (e->type == SDL_QUIT) {
        quit = true;
    }

    rubyChoice.buttonHandler(e);
    if (rubyChoice.clickedOn == true && tsToMapTransition == false) {
        rubyChoice.clickedOn = false;
        Mix_PlayChannel(-1, clickedOnSound, 0);
        mainPlayer.setPlayerGender(0);
        tsToMapTransition = true;
        return;
    }

    sapphireChoice.buttonHandler(e);
    if (sapphireChoice.clickedOn == true && tsToMapTransition == false) {
        sapphireChoice.clickedOn = false;
        Mix_PlayChannel(-1, clickedOnSound, 0);
        mainPlayer.setPlayerGender(1);
        tsToMapTransition = true;
        return;
    }
}

// SETUP SCREEN BUTTONS

SSButton::SSButton() {
    buttonTexture = NULL;
    buttonDest = {0, 0, 0, 0};
    currentButtonFrame = 0;
}

SSButton::~SSButton() {
    freeButton();
}

void SSButton::initSSB(const char* path, int x, int y, int w, int h, int BW, int BH) {
    freeButton();
    buttonTexture = IMG_LoadTexture(RenderWindow::renderer, path);
    buttonDest = {x, y, w, h};
    for (int i = 0; i < 3; i++) {
        buttonFrame[i] = {i*BW, 0, BW, BH};
    }
}

void SSButton::freeButton() {
    if (buttonTexture != NULL) SDL_DestroyTexture(buttonTexture);
    buttonTexture = NULL;
    clickedOn = false;
    currentButtonFrame = 0;
}

void SSButton::drawButton() {
    SDL_RenderCopy(RenderWindow::renderer, buttonTexture, &buttonFrame[currentButtonFrame], &buttonDest);
}

void SSButton::buttonHandler(SDL_Event* e) {
    if (e->type == SDL_MOUSEMOTION or e->type == SDL_MOUSEBUTTONDOWN or e->type == SDL_MOUSEBUTTONUP) {
        int x, y;
        eventMousePosition(*e, x, y);

        bool inside = true;
        if (x < buttonDest.x) inside = false;
        else if (x > buttonDest.x + buttonDest.w) inside = false;
        else if (y < buttonDest.y) inside = false;
        else if (y > buttonDest.y + buttonDest.h) inside = false;

        if (inside == false) {
            currentButtonFrame = 0;
        } else {
            switch (e->type) {
                case SDL_MOUSEMOTION:
                currentButtonFrame = 1;
                break;

                case SDL_MOUSEBUTTONDOWN:
                currentButtonFrame = 2;
                break;

                case SDL_MOUSEBUTTONUP:
                currentButtonFrame = 1;
                clickedOn = true;
                break;

                default:
                break;
            }
        }
    }
}