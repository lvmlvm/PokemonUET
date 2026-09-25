#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string>
using namespace std;

class Text {

public:
	Text();
	~Text();
	void freeText();

	void createFont(const char* fontPath, int fontSize);

	void display(int x, int y, SDL_Renderer* renderer);

	void textInit(SDL_Renderer* renderer, const char* message, const SDL_Color& color, int rightEdge = 753);

	SDL_Texture* loadFont(SDL_Renderer* renderer, const char* message, const SDL_Color& color, int rightEdge);

private:
	TTF_Font* textFont;
	SDL_Texture* TextTexture;
	SDL_Rect TextRect;
};

class dialogueBox {
public:
	dialogueBox();
	~dialogueBox();

	void freeDialogueBox();
	void initDialogueBox(SDL_Renderer* renderer, const char* path);
	void renderDialogueBox(SDL_Renderer* renderer, SDL_Rect* clip);
private:
	SDL_Texture* dialogueBoxTexture;
};

class MenuButton {
	private:
		SDL_Texture* buttonTexture;
	public:
		bool clickedOn = false;
		SDL_Rect buttonDest;
		MenuButton();
		~MenuButton();
		void initMB(const char* path, int x, int y);
		void freeButton();
		void drawButton();
        void buttonHandler();
};

class MenuPokemonSelectionScreen {
	private:
		SDL_Texture* pokePartyScreenText;
		SDL_Rect pokePartyScreenRect;

		SDL_Texture* canBattleTexture;
		SDL_Texture* cannotBattleTexture;

		SDL_Texture* HPBarTexture;
		SDL_Rect pokeBallRect[3];
		SDL_Rect hpBarRect[3];

		SDL_Texture* hpColor;
		SDL_Rect pokemonHP[3];
		Text pokeTextHPs[3];

		Text pokeNames[3];
	public:
		MenuButton backButton;

		MenuPokemonSelectionScreen();
		~MenuPokemonSelectionScreen();
		void freeMenuSelectionScreen();
		void initMenuSelectionScreen();
		void updateMenuSelectionScreen();
		void display();
};

class gameMenu {
	private:
		SDL_Texture* menuTexture;
		MenuButton buttons[4];
		Text highScore;
		SDL_Rect menuDest;
		MenuPokemonSelectionScreen menuSelScreen;
		bool inPokemonView, inHighScoreView;
	public:
		gameMenu();
		~gameMenu();
		void initMenu();
		void freeMenu();
		void centralMenuInputProcess();
		void drawMenu();
};