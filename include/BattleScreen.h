#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <vector>
#include <string>
#include "otherGraphics.h"
#include "RenderWindow.h"
#include "mPlayer.h"

class BattleScreenButton {
	private:
		SDL_Texture* buttonTexture;
		SDL_Rect buttonFrames[3];
		SDL_Rect buttonDest;
		int currentButtonFrame;
	public:
		Text moveNames;

		bool clickedOn = false;
		BattleScreenButton();
		~BattleScreenButton();
		void initBSB(const char* path, int x, int y, int BH, int BW, int imgWidth, int imgHeight, bool isMoveButton = false);
		void freeButton();
		void drawButton(bool isMoveButton = false);
        void buttonHandler();
		void moveButtonHandler(int buttonNum);
};

class PokemonSelectionScreen {
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

		Text whichPoke;
		Text pokeNames[3];

	public:
		BattleScreenButton backButton;
		BattleScreenButton pokemonSelectionButton[3];

		PokemonSelectionScreen();
		~PokemonSelectionScreen();
		void freeSelectionScreen();
		void initSelectionScreen(mPlayer* player);
		void updateSelectionScreen(mPlayer* player);
		void display();
};

class BattleScreen {
	private:
		mPlayer* battlePlayer;
		Trainer* battleOpponent;

		Pokemon* currentPlayerPokemon;
		Pokemon* currentOpponentPokemon;

		SDL_Texture* battleBackground;
		SDL_Texture* battleCircle;
		SDL_Texture* grayInputBox;

		SDL_Texture* playerTexture;
		SDL_Texture* opponentTexture;

		SDL_Texture* playerHPBar;
		SDL_Texture* oppoHPBar;
		SDL_Rect playerHPRect, oppoHPRect;
		Text playerPokeName, oppoPokeName;

		SDL_Texture* HPColor;
		SDL_Rect currPlayHP, currOppoHP;

		SDL_Texture* playerPokeTexture;
		SDL_Texture* opponentPokeTexture;
		
		SDL_Rect playerPokeRect, oppoPokeRect;

		SDL_Rect playerTextureFrames[4];
		SDL_Rect playerCircle, opponentCircle, grayBoxRect, playerSpriteBox, opponentSpriteBox;

		BattleScreenButton fightButton, pokemonButton, retireButton, backButton;
		BattleScreenButton moveButtons[4];
		
		Text bd_Text;

		PokemonSelectionScreen selScreen;
		Mix_Chunk *effectiveSFX, *notEffectiveSFX, *superEffectiveSFX, *pokemonFainted;
	public:
		vector<std::string> battleDialogues;
		vector<std::string> turnActionQueue;

		bool inAnim0 = true, moveAnim = false,
			fightScreen = false, moveScreen = false, inSelectionScreen = false, 
			showPHPBar = false, showOHPBar = false,
			startingBattle = true;
		string buffer;
		BattleScreen();
		~BattleScreen();
		void initBattleScreen(mPlayer* player, Trainer* opponent);
		void freeBattleScreen();
		void drawBattleScreen(bool fMtB, bool fBtM);
		void centralBattleProcess(SDL_Event* e);

		void init(mPlayer* player, Trainer* opponent);
		void updateScreen(Pokemon &my, Pokemon &op);
		int menuInput(Pokemon &my);
		int moveInput(Pokemon &my);
		void printText(string s);
		void oSentPkm(Pokemon &my,Pokemon &op);
		void pSentPkm(Pokemon &my,Pokemon &op);

		const std::string& actionAt(unsigned int i) const;   // "" when out of range
		const std::string& dialogueAt(unsigned int i) const; // "" when out of range

		void localTurnHandler(int move); // HANDLES TURN OF ATTACKING BETWEEN POKEMONS
		void localSwitchPokemonHandler(int selPoke); // HANDLES THE ACT OF PLAYER SWITCHING BETWEEN POKEMONS MID-BATTLE
		Pokemon* getCurrentPlayerPokemon() {return currentPlayerPokemon;}
};