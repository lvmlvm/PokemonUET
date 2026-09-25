#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include <vector>

class NPC {
private:
    int npcXCoords, npcYCoords; // NPC's Coordinates
    int faceDirection; // NPC's facing direction. 0 = SOUTH, 1 = EAST, 2 = NORTH, 3 = WEST
    SDL_Texture* npcTexture; // NPC's texture
    SDL_Rect walkFrames[16]; // Basic walk frame
    std::vector<std::string> dialogueTexts; // Dialogue text
    std::vector<std::string> preBattleTexts; // Pre-battle text (only used if istrainer = true)
    bool isTrainer; // If true then will generate a random battle after the dialouge
    int hasBattled = 2; // 0 = has not battled, 1 = has battled, 2 = not a trainer
    std::string trainerName, trainerPath;
public:
    NPC();
    ~NPC();
    void initNPC(int x, int y, int face, const char* texturePath, bool canBattle = false);
    void changeFacingDirection(int face);
    bool talkNPC(int playerFace);
    void drawNPC(int camX, int camY);
    int getX() {return npcXCoords;}
    int getY() {return npcYCoords;}
    int getFaceDirection() {return faceDirection;}
    bool getTrainerStatus() {return isTrainer;}
    int getBattleStatus() {return hasBattled;}
    void startBattle();
    void initDialogue(std::string nextSentence);
    void initPreBattleDialogue(std::string nextSentence);
    std::string getCurrentSentence();
    int getCurrentSentenceID();
    void changeBattleStatus(int status) {hasBattled = status;}

    void setTrainerName(std::string name) {trainerName = name;}
    void setTrainerSprite(std::string path) {trainerPath = path;}
    std::string getTrainerName() {return trainerName;}
    std::string getTrainerSprite() {return trainerPath;}
};
