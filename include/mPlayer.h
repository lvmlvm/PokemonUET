#pragma once

#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include "otherGraphics.h"
#include "Pokemon.h"

extern Pokemon pokemon[];

struct HighScoreListNode {
    HighScoreListNode* nextHighScore;
    int score;

    HighScoreListNode() {
        nextHighScore = NULL;
        score = 0;
    }

    HighScoreListNode(int _score) {
        nextHighScore = NULL;
        score = _score;
    }
};

class HighScoreList {
private:
    HighScoreListNode* head;
    SDL_Texture* highScoreScreenText;
    SDL_Rect highScoreScreenRect;
    Text highScoreTexts[5];    

public:
    HighScoreList();
    ~HighScoreList();

    MenuButton backButton;

    void initHighScoreList();
    void freeHighScoreList();
    void loadHighScoreList(int highScoreList[]);
    void printHighScoreList();  
    void saveHighScoreList(std::ofstream& saveStream);  
    void updateHighScoreList(int newHighScore);
    void resetHighScoreList();

    void drawHighScoreScreen();
};

class mPlayer {
    private:
        int gender;
        std::string name;
        int currentMap;
        int xCoords, yCoords;
        int faceDirection;
        SDL_Texture* playerTexture;
        SDL_Rect walkFrames[32];
        int currentHighScore;
        
    public:
        mPlayer();
        ~mPlayer();

        Pokemon party[3];
        int activePokemonCount;
        HighScoreList playerScoreList;

        bool loadPlayerData();
        bool savePlayerData();
        void resetPlayerData();

        int getGender();
        std::string getPlayerName();
        int getCurrentMap();
        int getXCoords();
        int getYCoords();
        int getFacingDirection();
        SDL_Texture* getPlayerTexture();
        SDL_Rect* getCurrentFrame();

        void setPlayerGender(int gender);
        void changeFacingDirect(int direct);
        void initPlayerTexture();
        void setPlayerCoords(int x, int y, int mapID);
        void setFacingDirection(int face) {faceDirection = face;}

        void renderStandingPlayer();
        void renderMovingPlayer();
        void renderRunningPlayer();

        int getCurrentHighScore() {return currentHighScore;}
        void updateCurrentHighScore(int newXP) {currentHighScore += newXP;}
        void resetCurrentHighScore() {currentHighScore = 0;}
};