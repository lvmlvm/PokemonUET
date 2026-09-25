#pragma once
#include "Pokemon.h"
#include "RenderWindow.h"
#include "titleScreen.h"
#include "music.h"
#include "Tiling.h"
#include "map.h"
#include "mPlayer.h"
#include "camera.h"
#include "Battle.h"
#include "BattleScreen.h"
#include "NPCs.h"
#include "otherGraphics.h"

using namespace std;

extern bool debugMode;

extern RenderWindow renderWindow;
extern SDL_Event e;

extern TitleScreen gameTitleScreen;
extern SDL_Texture* blackTransitionTexture;
const int MAP_COUNT = 12;
extern string gameMaps[];
extern string gameTileSets[];
extern string gameThemes[];

extern double themeRepeats[];
extern bool mapOverlays[];

extern string Type[];
extern Move moves[];
extern PokemonData pokemonData[];
extern int psize;
extern double typeEffectiveness[][19];

extern SDL_Rect dBoxClip;
extern dialogueBox d_box;
extern Text d_text;

extern Mix_Chunk* changeMap;
extern Mix_Chunk* aButton;
extern Mix_Chunk* gameSaved;
extern Mix_Chunk* startMenuSound;
extern Mix_Chunk* deniedSound;
extern Mix_Chunk* clickedOnSound;

extern Map* playerMap;
extern Music gameMusic;

extern mPlayer mainPlayer;
extern gameCam mainCamera;

extern SetupScreen mainSetup;
extern BattleScreen mainBattle;
extern gameMenu mainMenu;

// GAME STATES

extern bool tsToMapTransition;
extern bool beginMapToMapTransition;
extern bool finishMapToMapTransition;
extern int transitionTransparency;

extern bool startTSToSetupTransition;
extern bool finishTSToSetupTransition;

extern bool beginMapToBattleTransition;
extern bool finishMapToBattleTransition;
extern bool beginBattleToMapTransition;
extern bool finishBattleToMapTransition;

extern bool hasSaveFile;
extern bool quit;

extern bool inTitleScreen; // SET TO FALSE TO SKIP TITLE SCREEN FOR FASTER DEBUG
extern bool inSetupScreen;
extern bool inBattle;
extern bool inDialogue;
extern bool inMenu;

extern bool playerIsRunning;

extern Trainer defaultOppo;