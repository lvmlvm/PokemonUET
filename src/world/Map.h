#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <vector>

#include "world/Npc.h"
#include "core/Music.h"
#include "world/Camera.h"
#include "world/Tiling.h"

class WarpTile {
    private:
        int tileX, tileY;
        int destMap;
        int destX, destY;
    public:
        WarpTile(int _tileX, int _tileY, int _destMap, int _destX, int _destY);
        ~WarpTile();
        int getX() {return tileX;}
        int getY() {return tileY;}
        int getDestMap() {return destMap;}
        int getDestX() {return destX;}
        int getDestY() {return destY;}
};

class InterTile {
    private:
        int tileX, tileY;
        unsigned int dialogueCursor; // 1-based index of the sentence being shown, 0 = not talking
    public:
        std::vector<std::string> dialogueTexts;
        InterTile(int _tileX, int _tileY);
        ~InterTile();
        int getX() {return tileX;}
        int getY() {return tileY;}
        void initTileDialogue(std::string nextSentence);
        bool talkTile();
        std::string getInterCurrentSentence();
        int getInterCurrentSentenceID();
        int getInterDialogueSize() {return dialogueTexts.size();}
};

class Map {
    private:
        int mapID;
        int** map; //Tile map
        int** tilePropMap; //Collision, Events Map
        int** tileOverlayMap; // Stuffs that are drawn on top of players and NPCs
        int mapWidth;
        int mapHeight;
        TileSheet mapSheet;
        std::vector<NPC*> mapNPCs;
        std::vector<NPC*> NPCsinFront; // Only used for rendering NPCs that are in front of the players
        std::vector<WarpTile*> mapWarpTiles;
        std::vector<InterTile*> mapInterTiles;
    public:
        // Music mapTheme;
        Map();
        ~Map();
        void freeMap();
        void loadMap(const char* path, const char* sheetPath, const char* musicPath, double repeatP = 0.0, bool hasOverlay = false);
        void drawMap(gameCam* camera);
        void drawNPCs(gameCam* camera);
        void drawFrontNPCs(gameCam* camera);
        // void playMapTheme();
        void initOverlayElements(const char* path);
        void freeOverlayElements();
        void drawOverlay(gameCam* camera);
        int getMapID() {return mapID;}
        int getMapWidth();
        int getMapHeight();
        void popLastNPC();
        int** getCollisionMap();
        NPC* getNearbyNPC(int pCX, int pCY, int playerFace);
        WarpTile* getNearbyWarpTile(int pCX, int PCY, int playerFace);
        InterTile* getNearbyInterTile(int pCX, int PCY, int playerFace);
};