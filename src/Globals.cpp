#include "Globals.h"

bool debugMode=false;

RenderWindow renderWindow;

SDL_Event e;

TitleScreen gameTitleScreen;
SDL_Texture* blackTransitionTexture;

std::string gameMaps[] = {
    "res/map/g2.map",
    "res/map/e3.map",
    "res/map/e3i.map",
    "res/map/g2i.map",
    "res/map/e3i_2.map",
    "res/map/chal5.map",
    "res/map/chal6.map",
    "res/map/chal7.map",
    "res/map/chal8.map",
    "res/map/chal9.map",
    "res/map/chal10.map",
    "res/map/chal11.map"
};
std::string gameTileSets[] = {
    "res/tileset/g2o_tiles.png",
    "res/tileset/e3o_tiles.png",
    "res/tileset/e3i_tiles.png",
    "res/tileset/g2i_tiles.png",
    "res/tileset/e3i_2_tiles.png",
    "res/tileset/chal5_tiles.png",
    "res/tileset/chal6_tiles.png",
    "res/tileset/chal7_tiles.png",
    "res/tileset/chal8_tiles.png",
    "res/tileset/chal9_tiles.png",
    "res/tileset/chal10_tiles.png",
    "res/tileset/chal11_tiles.png"
};
std::string gameThemes[] = {
    "res/music/g2o_theme.mp3",
    "res/music/e3o_theme.mp3",
    "res/music/e3i_theme.mp3",
    "res/music/e3i_theme.mp3",
    "res/music/e3i_theme.mp3",
    "res/music/e3i_theme.mp3",
    "res/music/e3i_theme.mp3",
    "res/music/e3i_theme.mp3",
    "res/music/e3i_theme.mp3",
    "res/music/e3i_theme.mp3",
    "res/music/e3i_theme.mp3",
    "res/music/e3i_theme.mp3"
};
double themeRepeats[] = {
    8.85,
    30.555,
    0.77,
    0.77,
    0.77,
    0.77,
    0.77,
    0.77,
    0.77,
    0.77,
    0.77,
    0.77
};
bool mapOverlays[] = {
    false,
    false,
    true,
    true,
    true,
    false,
    false,
    false,
    false,
    false,
    false,
    false
};
std::string Type[]= {
    "NULL", // 0
	"Normal", // 1
	"Fire", // 2
	"Water", // 3
	"Electric", // 4
	"Grass", // 5
	"Ice", // 6
	"Fighting", // 7
	"Poison", // 8
	"Ground", // 9
	"Flying", // 10
	"Psychic", // 11
	"Bug", // 12
	"Rock", // 13
	"Ghost", // 14
	"Dragon", // 15
	"Dark", // 16
	"Steel", // 17
	"Fairy" // 18
};

Move moves[]=
{
	{"Body Slam",1,85,15},
	{"Flamethrower",2,90,15},
    {"Surf",3,90,15},
    {"Thunderbolt",4,90,15},
    {"Leaf Blade",5,90,15},
    {"Ice Beam",6,90,10},
    {"Sacred Sword",7,90,15},
    {"Sludge Bomb",8,90,10},
    {"Earthquake",9,100,10},
    {"Drill Peck",10,80,20},
    {"Psychic",11,90,10},
    {"Bug Buzz",12,90,10},
    {"Power Gem",13,80,20},
    {"Shadow Ball",14,80,20},
    {"Dragon Pulse",15,85,10},
    {"Darkest Lariat",16,85,10},
    {"Iron Head",17,80,15},
    {"Moon Blast",18,95,15}
};
PokemonData pokemonData[]=
{
    {"MissingNo.", 0, 0, 1, 0, 0, 0, {moves,moves,moves,moves}},
    {"Tauros", 1, 0, 135, 105, 75, 115, {moves,moves+6,moves+8,moves+15}}, 
    {"Snorlax", 1, 0, 220, 70, 115, 35, {moves+5,moves+1,moves+3,moves+10}},
    {"Charizard", 2, 10, 138, 114, 83, 105, {moves+1,moves+8,moves+3,moves+6}}, 
    {"Torkoal", 2, 0, 130, 90, 145, 25, {moves+1,moves+8,moves+16,moves}}, 
    {"Gyarados", 3, 10, 155, 130, 84, 86, {moves+2,moves+8,moves+5,moves+12}}, 
    {"Milotic", 3, 0, 155, 65, 130, 86,{moves+2,moves+5,moves+16,moves}}, 
    {"Electivire", 4, 0, 135, 128, 72, 100,{moves+3,moves+6,moves+8,moves+5}}, 
    {"Magnezone", 4, 17, 130, 75, 120, 65, {moves+16,moves+3,moves+5,moves}},
    {"Exeggutor", 5, 11, 155, 130, 80, 60, {moves+10,moves+4,moves+1,moves}},
    {"Virizion", 5, 7, 151, 95, 134, 113, {moves+6,moves+4,moves+12,moves+11}}, 
    {"Mamoswine", 6, 9, 170, 135, 65, 85, {moves+8,moves+5,moves+12,moves+6}},
    {"Lapras", 6, 3, 190, 90, 100, 65, {moves+2,moves+5,moves+3,moves}},
    {"Lucario", 7, 17, 130, 120, 75, 95, {moves+16,moves+6,moves+5,moves+8}},
    {"Infernape", 7, 2, 136, 109, 76, 113, {moves+1,moves+6,moves+3,moves}},
    {"Gengar", 8, 14, 120, 135, 65, 115, {moves+13,moves+7,moves+3,moves+6}},  
    {"Venusaur", 8, 5, 140, 87, 105, 85, {moves+4,moves+7,moves+8,moves}},
    {"Swampert", 9, 3, 160, 115, 95, 65, {moves+2,moves+8,moves+12,moves}},
    {"Golem", 9, 13, 140, 60, 135, 50, {moves+12,moves+8,moves+15,moves}},
    {"Archeops", 10, 13, 135, 145, 70, 115, {moves+8,moves+9,moves+12,moves+16}},
    {"Togekiss", 10, 18, 145, 125, 100, 85, {moves+6,moves+9,moves+17,moves}},
    {"Alakazam", 11, 0, 105, 140, 50, 125, {moves+13,moves+10,moves+4,moves+1}},
    {"Mr. Mime", 11, 18, 100, 50, 125, 95, {moves+5,moves+10,moves+3,moves+4}},
    {"Heracross", 12, 7, 140, 130, 80, 90, {moves+6,moves+11,moves+12,moves+15}},
    {"Shuckle", 12, 13, 80, 15, 235, 10, {moves+16,moves+11,moves+12,moves}},
    {"Crustle", 13, 12, 130, 70, 130, 50, {moves+11,moves+12,moves+8,moves}},
    {"Sudowoodo", 13, 0, 130, 35, 120, 35, {moves+4,moves+12,moves+15,moves}},
    {"Chandelure", 14, 3, 120, 150, 60, 85, {moves+1,moves+13,moves+4,moves+5}},
    {"Dusknoir", 14, 0, 105, 105, 140, 50, {moves+6,moves+13,moves+5,moves}},
    {"Garchomp", 15, 9, 168, 135, 90, 107, {moves+8,moves+14,moves+12,moves+1}},
    {"Dragonite", 15, 10, 151, 139, 100, 85, {moves+1,moves+14,moves+3,moves}},
    {"Absol", 16, 0, 125, 125, 70, 80, {moves+6,moves+15,moves+12,moves+10}},
    {"Umbreon", 16, 0, 155, 65, 135, 70, {moves+10,moves+15,moves+6,moves}},
    {"Excadrill", 17, 9, 170, 140, 65, 93, {moves+8,moves+16,moves+12,moves+11}},
    {"Steelix", 17, 9, 135, 60, 205, 35, {moves+8,moves+16,moves+12,moves+15}},
    {"Gardevoir", 18, 11, 128, 130, 70, 85, {moves+10,moves+17,moves+3,moves+13}},
    {"Clefable", 18, 0, 155, 100, 78, 65, {moves+10,moves+17,moves+1,moves}}
};

int psize=sizeof(pokemonData)/sizeof(pokemonData[0]);

double typeEffectiveness[19][19]={
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,0.5,0,1,1,0.5,1},
    {1,1,0.5,0.5,1,2,2,1,1,1,1,1,2,0.5,1,0.5,1,2,1},
    {1,1,2,0.5,1,0.5,1,1,1,2,1,1,1,2,1,0.5,1,1,1},
    {1,1,1,2,0.5,0.5,1,1,1,0,2,1,1,1,1,0.5,1,1,1},
    {1,1,0.5,2,1,0.5,1,1,0.5,2,0.5,1,0.5,2,1,0.5,1,0.5,1},
    {1,1,0.5,0.5,1,2,0.5,1,1,2,2,1,1,1,1,2,1,0.5,1},
    {1,2,1,1,1,1,2,1,0.5,1,0.5,0.5,0.5,2,0,1,2,2,0.5},
    {1,1,1,1,1,2,1,1,0.5,0.5,1,1,1,0.5,0.5,1,1,0,2},
    {1,1,2,1,2,0.5,1,1,2,1,0,1,0.5,2,1,1,1,2,1},
    {1,1,1,1,0.5,2,1,2,1,1,1,1,2,0.5,1,1,1,0.5,1},
    {1,1,1,1,1,1,1,2,2,1,1,0.5,1,1,1,1,0,0.5,1},
    {1,1,0.5,1,1,2,1,0.5,0.5,1,0.5,2,1,1,0.5,1,2,0.5,0.5},
    {1,1,2,1,1,1,2,0.5,1,0.5,2,1,2,1,1,1,1,0.5,1},
    {1,0,1,1,1,1,1,1,1,1,1,2,1,1,2,1,0.5,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,0.5,0},
    {1,1,1,1,1,1,1,0.5,1,1,1,2,1,1,2,1,0.5,1,0.5},
    {1,1,0.5,0.5,0.5,1,2,1,1,1,1,1,1,2,1,1,1,0.5,2},
    {1,1,0.5,1,1,1,1,2,0.5,1,1,1,1,1,1,2,2,0.5,1}
};

SDL_Rect dBoxClip = { 10, 515, 812, 179 };
dialogueBox d_box;
Text d_text;

Mix_Chunk* changeMap;
Mix_Chunk* aButton;
Mix_Chunk* gameSaved;
Mix_Chunk* startMenuSound;
Mix_Chunk* deniedSound;
Mix_Chunk* clickedOnSound;

Map* playerMap;
Music gameMusic;

mPlayer mainPlayer;
gameCam mainCamera;

SetupScreen mainSetup;
BattleScreen mainBattle;
gameMenu mainMenu;

// GAME STATES

bool tsToMapTransition = false;
bool beginMapToMapTransition = false;
bool finishMapToMapTransition = false;
int transitionTransparency = 0;

bool startTSToSetupTransition = false;
bool finishTSToSetupTransition = false;

bool beginMapToBattleTransition = false;
bool finishMapToBattleTransition = false;
bool beginBattleToMapTransition = false;
bool finishBattleToMapTransition = false;

bool hasSaveFile = true;
bool quit = false;

bool inTitleScreen = true; // SET TO FALSE TO SKIP TITLE SCREEN FOR FASTER DEBUG
bool inSetupScreen = false;
bool inBattle = false;
bool inDialogue = false;
bool inMenu = false;

bool playerIsRunning = false;

Trainer defaultOppo;