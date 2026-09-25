#include <iostream>
#include <fstream>

#include "Variables.h"
#include "NPCs.h"
#include "map.h"
#include "RenderWindow.h"
#include "Tiling.h"
#include "rng.h"

using namespace std;

bool mapDebug = false;

TileSheet overlayElements;

Map::Map() {
    map = NULL;
    tilePropMap = NULL; // 0 = walkable, 1 = not walkable, 2 = warp tile, 3 = interactable, 4 = hasNPC
    tileOverlayMap = NULL;
    mapWidth = 0;
    mapHeight = 0;
}

Map::~Map() {
    freeMap();
}

void Map::freeMap() {   
    // FREEING THE COLLISION MAP
    for (int i = 0; i < mapHeight; i++) {
        delete[] tilePropMap[i];
    }
    delete[] tilePropMap;
    tilePropMap = NULL;
    if (mapDebug == true) cerr << "Collision Map Freed Properly\n";

    // FREEING THE TEXTURE MAP
    for (int i = 0; i < mapHeight; i++) {
        delete[] map[i];
    }
    delete[] map;
    map = NULL;
    if (mapDebug == true) cerr << "Texture Map Freed Properly\n";

    // FREEING THE OVERLAY MAP (IF THE MAP HAS ONE)
    if (tileOverlayMap != NULL) {
        for (int i = 0; i < mapHeight; i++) {
            delete[] tileOverlayMap[i];
        }
        delete[] tileOverlayMap;
        tileOverlayMap = NULL;
        if (mapDebug == true) cerr << "Overlay Map Freed Properly\n";
    }

    // FREEING THE MAP'S NPCS
    NPCsinFront.clear();
    for (unsigned int i = 0; i < mapNPCs.size(); i++) {
        delete mapNPCs[i];
    }
    mapNPCs.clear();
    if (mapDebug == true) cerr << "Map NPCs Freed Properly\n";

    // FREEING THE MAP'S WARPTILES
    for (unsigned int i = 0; i < mapWarpTiles.size(); i++) {
        delete mapWarpTiles[i];
    }
    mapWarpTiles.clear();
    if (mapDebug == true) cerr << "Map Warp Tiles Freed Properly\n";

    // FREEING THE MAP'S INTERACTIBLE TILES
    for (unsigned int i = 0; i < mapInterTiles.size(); i++) {
        delete mapInterTiles[i];
    }
    mapInterTiles.clear();
    if (mapDebug == true) cerr << "Map Inter Tiles Freed Properly\n";

    mapWidth = 0;
    mapHeight = 0;
    mapSheet.freeTileSheet();
    if (mapDebug == true) cerr << "Tilesheet Freed Successfully\n";

    if (mapDebug == true) cerr << "Map Freed Successfully\n\n";
}

void Map::loadMap(const char* path, const char* sheetPath, const char* musicPath, double repeatP, bool hasOverlay) {
    // Load the level
    ifstream inputmap(path);
    string temp;
    inputmap >> mapID;
    inputmap >> mapHeight;
    inputmap >> mapWidth;
    if (!inputmap or mapHeight <= 0 or mapWidth <= 0) {
        cerr << "Failed to load map " << path << '\n';
        mapHeight = mapWidth = 0;
        return;
    }

    // Load in the tile texture data
    map = new int*[mapHeight];
    for (int i = 0; i < mapHeight; i++) {
        map[i] = new int[mapWidth];
    }
    for (int i = 0; i < mapHeight; i++) {
        for (int j = 0; j < mapWidth; j++) {
            inputmap >> map[i][j];
        }
    }
    if (mapDebug == true) cerr << "Tile texture data loaded\n";

    // Flush the end tile texture flag
    inputmap >> temp;

    // Load in the tile collision data
    tilePropMap = new int*[mapHeight];
    for (int i = 0; i < mapHeight; i++) {
        tilePropMap[i] = new int[mapWidth];
    }
    for (int i = 0; i < mapHeight; i++) {
        for (int j = 0; j < mapWidth; j++) {
            inputmap >> tilePropMap[i][j];
        }
    }
    if (mapDebug == true) cerr << "Tile collision data loaded\n";

    // Flush the tile collision flag
    inputmap >> temp;

    // Reads the tile overlay map if it exists
    if (hasOverlay == true) {
        tileOverlayMap = new int*[mapHeight];
        for (int i = 0; i < mapHeight; i++) {
            tileOverlayMap[i] = new int[mapWidth];
        }
        for (int i = 0; i < mapHeight; i++) {
            for (int j = 0; j < mapWidth; j++) {
                inputmap >> tileOverlayMap[i][j];
            }
        }
        if (mapDebug == true) cerr << "Tile overlay data loaded\n";

        // Flush the tile overlay flag
        inputmap >> temp;
    }

    // Reads the NPC data
    int i = 0;
    string nextNPC;
    while (i < 1) {
        i++;
        inputmap >> nextNPC;
        if (nextNPC != "MAP_NEXT_NPC" or nextNPC == "NPC_DATA_STOP") {
            break;
        } else {
            int npcX, npcY, face; 
            bool isTrainer;
            string npcTextPath;
            inputmap >> npcX >> npcY >> face >> isTrainer >> npcTextPath;

            NPC* newNPC = new NPC;
            newNPC->initNPC(npcX, npcY, face, npcTextPath.c_str(), isTrainer);

            if (isTrainer == true) {
                string preBattleSentence;
                while (preBattleSentence != "NPC_PREBATTLE_END") {
                    getline(inputmap, preBattleSentence);
                    if (preBattleSentence != "NPC_PREBATTLE_END" and preBattleSentence != "") {
                    newNPC->initPreBattleDialogue(preBattleSentence);
                    }
                }

                string trainerName, trainerPath;
                getline(inputmap, trainerName);
                getline(inputmap, trainerPath);
                newNPC->setTrainerName(trainerName);
                newNPC->setTrainerSprite(trainerPath);
            }
            
            string dialogueSentence;
            while (dialogueSentence != "NPC_DIALOGUE_END") {
                getline(inputmap, dialogueSentence);
                if (dialogueSentence != "NPC_DIALOGUE_END" and dialogueSentence != "") {
                    newNPC->initDialogue(dialogueSentence);
                }
            }

            mapNPCs.push_back(newNPC);

            tilePropMap[npcY][npcX] = 4;
            i--;
        }
    }

    if (mapID == 3 and (mainPlayer.party[0].data - pokemonData) == 0) { // EDGE CASE FOR E3 BUILDING. BLOCKS THE PLAYER FROM ENTERING IF THEY HAVE NO POKEMON
        NPC* newNPC1 = new NPC;
        newNPC1->initNPC(27, 9, 0, "res/npcsprite/npcSprite6.png");
        newNPC1->initDialogue("I'm sorry, but you must first acquire some Pokemons before you may take the Pokemon UET Challenge.");
        newNPC1->initDialogue("To get your Pokemons, go to the Student Button Room in the E3 Building.");

        NPC* newNPC2 = new NPC;
        newNPC2->initNPC(28, 9, 0, "res/npcsprite/npcSprite6.png");
        newNPC2->initDialogue("I'm sorry, but you must first acquire some Pokemons before you may take the Pokemon UET Challenge.");
        newNPC2->initDialogue("To get your Pokemons, go to the Student Button Room in the E3 Building.");

        mapNPCs.push_back(newNPC1);
        mapNPCs.push_back(newNPC2);
        tilePropMap[9][27] = 4;
        tilePropMap[9][28] = 4;
    }

    if (mapID >= 5 and mapID <= 11) {
        int randDestMap = randomInt(6, 11);

        WarpTile* newWarpTile = new WarpTile(12, 7, randDestMap, 12, 22);
        mapWarpTiles.push_back(newWarpTile);

        NPC* newNPC1 = new NPC;
        newNPC1->initNPC(12, 8, 0, "res/npcsprite/npcSprite6.png");
        newNPC1->initDialogue("You must battle and win vs the Trainer to advance to the next room!");

        mapNPCs.push_back(newNPC1);
        tilePropMap[8][12] = 4;
    }

    if (mapDebug == true) cerr << "NPC data loaded\n";

    // WARP AND INTER TILE LOADINGING

    i = 0;
    string nextWarpTile;
    while (i < 1) {
        i++;
        inputmap >> nextWarpTile;
        if (nextWarpTile != "WARP_NEXT_TILE" or nextWarpTile == "WARP_DATA_STOP") {
            break;
        } else {
            int tileX, tileY, destMap, destX, destY;
            inputmap >> tileX >> tileY >> destMap >> destX >> destY;

            WarpTile* newWarpTile = new WarpTile(tileX, tileY, destMap, destX, destY);
            
            mapWarpTiles.push_back(newWarpTile);
            tilePropMap[tileY][tileX] = 2;
            i--;
        }
    }
    if (mapDebug == true) cerr << "Warp tile data loaded\n";

    i = 0;
    string nextInterTile;
    while (i < 1) {
        i++;
        inputmap >> nextInterTile;
        if (nextInterTile != "INTER_NEXT_TILE" or nextInterTile == "INTER_DATA_STOP") {
            break;
        } else {
            int tileX, tileY;
            inputmap >> tileX >> tileY;

            InterTile* newInterTile = new InterTile(tileX, tileY);

            string interSentence;
            while (interSentence != "INTER_DIALOGUE_END") {
                getline(inputmap, interSentence);
                if (interSentence != "INTER_DIALOGUE_END" and interSentence != "") {
                    newInterTile->initTileDialogue(interSentence);
                }
            }

            mapInterTiles.push_back(newInterTile);
            tilePropMap[tileY][tileX] = 3;
            i--;
        }
    }
    if (mapDebug == true) cerr << "Inter tile data loaded\n";

    inputmap.close();

    // Load the map's tilesheet
    mapSheet.loadTileSheet(sheetPath);
    // mapTheme.loadMusic(musicPath, repeatP);

    if (mapDebug == true) cerr << "Everything loaded\n----------------------------------------------\n";
}

void Map::drawMap(gameCam* camera) {
    for (int i = 0; i < mapHeight; i++) {
        for (int j = 0; j < mapWidth; j++) {
            Tile drawTile;
            drawTile.defineTile(&mapSheet, map[i][j]);
            SDL_Rect dstRect;
            dstRect.x = j*64 - camera->getCamX();
            dstRect.y = i*64 - camera->getCamY();
            dstRect.w = 64;
            dstRect.h = 64;
            SDL_RenderCopy(RenderWindow::renderer, mapSheet.getTileSheet(), drawTile.getClip(), &dstRect);
        }
    }
}

void Map::drawNPCs(gameCam* camera) {
    NPCsinFront.clear();
    for (unsigned int i = 0; i < mapNPCs.size(); i++) {
        if (camera->getCamY() + 64*5 < mapNPCs[i]->getY()*64) {
            NPCsinFront.push_back(mapNPCs[i]);
        } else {
            mapNPCs[i]->drawNPC(camera->getCamX(), camera->getCamY());
        }
    }
}

void Map::drawFrontNPCs(gameCam* camera) {
    for (unsigned int i = 0; i < NPCsinFront.size(); i++) {
        NPCsinFront[i]->drawNPC(camera->getCamX(), camera->getCamY());
    }
}

void Map::initOverlayElements(const char* path) {
    overlayElements.loadTileSheet(path);
}

void Map::freeOverlayElements() {
    overlayElements.freeTileSheet();
}

void Map::drawOverlay(gameCam* camera) {
    for (int i = 0; i < mapHeight; i++) {
        for (int j = 0; j < mapWidth; j++) {
            if (tileOverlayMap[i][j] != -1) {
                Tile overlayTile;
                overlayTile.defineTile(&overlayElements, tileOverlayMap[i][j]);
                SDL_Rect dstRect;
                dstRect.x = j*64 - camera->getCamX();
                dstRect.y = i*64 - camera->getCamY();
                dstRect.w = 64;
                dstRect.h = 64;
                SDL_RenderCopy(RenderWindow::renderer, overlayElements.getTileSheet(), overlayTile.getClip(), &dstRect);
            }
        }
    }
}

int Map::getMapWidth() {
    return mapWidth;
}

int Map::getMapHeight() {
    return mapHeight;
}

int** Map::getCollisionMap() {
    return tilePropMap;
}

void Map::popLastNPC() {
    if (mapNPCs.empty()) return;
    int lastNPC = mapNPCs.size() - 1;
    tilePropMap[mapNPCs[lastNPC]->getY()][mapNPCs[lastNPC]->getX()] = 0;
    delete mapNPCs[lastNPC];
    mapNPCs.pop_back();
}

NPC* Map::getNearbyNPC(int pCX, int pCY, int playerFace) {
    for (unsigned int i = 0; i < mapNPCs.size(); i++) {
        switch (playerFace) {
        case 0:
            if (mapNPCs[i]->getY() == pCY + 1 and mapNPCs[i]->getX() == pCX) return mapNPCs[i];
            break;
        case 1:
            if (mapNPCs[i]->getX() == pCX + 1 and mapNPCs[i]->getY() == pCY) return mapNPCs[i];
            break;
        case 2:
            if (mapNPCs[i]->getY() == pCY - 1 and mapNPCs[i]->getX() == pCX) return mapNPCs[i];
            break;
        case 3:
            if (mapNPCs[i]->getX() == pCX - 1 and mapNPCs[i]->getY() == pCY) return mapNPCs[i];
            break;
        default:
            break;
        }
    }
    
    return NULL;
}

WarpTile* Map::getNearbyWarpTile(int pCX, int pCY, int playerFace) {
    for (unsigned int i = 0; i < mapWarpTiles.size(); i++) {
        switch (playerFace) {
        case 0:
            if (mapWarpTiles[i]->getY() == pCY + 1 and mapWarpTiles[i]->getX() == pCX) return mapWarpTiles[i];
            break;
        case 1:
            if (mapWarpTiles[i]->getX() == pCX + 1 and mapWarpTiles[i]->getY() == pCY) return mapWarpTiles[i];
            break;
        case 2:
            if (mapWarpTiles[i]->getY() == pCY - 1 and mapWarpTiles[i]->getX() == pCX) return mapWarpTiles[i];
            break;
        case 3:
            if (mapWarpTiles[i]->getX() == pCX - 1 and mapWarpTiles[i]->getY() == pCY) return mapWarpTiles[i];
            break;
        default:
            break;
        }
    }
    
    return NULL;
}

InterTile* Map::getNearbyInterTile(int pCX, int pCY, int playerFace) {
    for (unsigned int i = 0; i < mapInterTiles.size(); i++) {
        switch (playerFace) {
        case 0:
            if (mapInterTiles[i]->getY() == pCY + 1 and mapInterTiles[i]->getX() == pCX) return mapInterTiles[i];
            break;
        case 1:
            if (mapInterTiles[i]->getX() == pCX + 1 and mapInterTiles[i]->getY() == pCY) return mapInterTiles[i];
            break;
        case 2:
            if (mapInterTiles[i]->getY() == pCY - 1 and mapInterTiles[i]->getX() == pCX) return mapInterTiles[i];
            break;
        case 3:
            if (mapInterTiles[i]->getX() == pCX - 1 and mapInterTiles[i]->getY() == pCY) return mapInterTiles[i];
            break;
        default:
            break;
        }
    }
    
    return NULL;
}

// WARP TILE CONFIGS

WarpTile::WarpTile(int _tileX, int _tileY, int _destMap, int _destX, int _destY) {
    tileX = _tileX;
    tileY = _tileY;
    destMap = _destMap;
    destX = _destX;
    destY = _destY;
}

WarpTile::~WarpTile() {
    tileX = 0;
    tileY = 0;
    destMap = 0;
    destX = 0;
    destY = 0;
}

// INTER TILE CONFIGS

InterTile::InterTile(int _tileX, int _tileY) {
    tileX = _tileX;
    tileY = _tileY;
    dialogueCursor = 0;
}

InterTile::~InterTile() {
    tileX = 0;
    tileY = 0;
    dialogueTexts.clear();
}

void InterTile::initTileDialogue(std::string nextSentence) {
    dialogueTexts.push_back(nextSentence);
}

bool InterTile::talkTile() {
    if (dialogueCursor < dialogueTexts.size()) {
        dialogueCursor++;
        return true;
    }
    else {
        dialogueCursor = 0;
        return false;
    }
}

std::string InterTile::getInterCurrentSentence() {
    if (dialogueCursor == 0 or dialogueCursor > dialogueTexts.size()) return "";
    return dialogueTexts[dialogueCursor - 1];
}

int InterTile::getInterCurrentSentenceID() {
    return dialogueCursor;
}