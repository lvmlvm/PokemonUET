#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include "ui/OtherGraphics.h"

class TitleScreenButton {
    private:
        SDL_Texture* buttonTexture;
        bool clickedOn = false;
        SDL_Rect buttonFrames[3];
        SDL_Rect buttonDest;
        int currentButtonFrame;
    public:
        TitleScreenButton();
        ~TitleScreenButton();
        void initTSBTexture(const char* path); // Initialize the buttons
        void freeButton();
        void setButtonDest(int x, int y);
        SDL_Rect* getButtonFrame();
        void drawButton();
        void buttonHandler(SDL_Event* e);
        bool isClicked();
        void resetClickState();
        SDL_Texture* getButtonTexture();
};

class SSButton {
	private:
		SDL_Texture* buttonTexture;
        SDL_Rect buttonDest;
        SDL_Rect buttonFrame[3];
        int currentButtonFrame;
	public:
		bool clickedOn = false;
		SSButton();
		~SSButton();
		void initSSB(const char* path, int x, int y, int w, int h, int BW, int BH);
        void freeButton();
		void drawButton();
        void buttonHandler(SDL_Event* e);
};

class TitleScreen {
    private:
        SDL_Texture* splashScreenTexture;
        SDL_Texture* titleScreenBackground;
        SDL_Texture* titleScreenLogo;
        SDL_Texture* helpScreenTexture;
        bool presentSaveFile;
        bool inHelpScreen;
        bool acceptInput;
    public:
        TitleScreenButton tsButtons[4];
        SSButton backButton;
        TitleScreen();
        ~TitleScreen();
        void freeTitleScreen();
        void initTitleScreen(bool hasSaveFile);
        void drawTitleScreen();
        void stopInputState();
        bool acceptInputState();
        void doButtonEvents(SDL_Event* e);
        void tsButtonInit(); // Place the buttons on the title screen
        void toggleHelpScreen() {inHelpScreen = true;}
};

class SetupScreen {
    private:
        Text chooseUrGender;
        SSButton rubyChoice;
        SSButton sapphireChoice;
    public:
        SetupScreen();
        ~SetupScreen();
        void initSetupScreen();
        void freeSetupScreen();
        void drawSetupScreen();
        void setupScreenInputProcess(SDL_Event* e);
};