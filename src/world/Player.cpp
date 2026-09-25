#include "world/Player.h"
#include "core/RenderWindow.h"
#include "battle/Battle.h"
#include "Globals.h"

#include <iostream>
#include <fstream>

SDL_Rect screenCenter = {(832-64)/2, (704-64)/2-24, 64, 88};
static int moveFrame = 0, walkFrame = 0;

mPlayer::mPlayer() {
    gender = 0; // -1 = UNDEFINED, 0 = MALE, 1 = FEMALE
    name = "Player";
    currentMap = 1; // 0 = G2 EXTERIOR, 1 = E3 EXTERIOR, 2 = E3 INTERIOR, 3 = G2 INTERIOR, 4 = STUDENT BUTTON ROOM , 5 -> 10 = CHALLENGE ROOMS
    xCoords = 20, yCoords = 10;
    faceDirection = 0; // 0 = SOUTH, 1 = EAST, 2 = NORTH, 3 = WEST
    playerTexture = NULL;
    currentHighScore = 0;

    for (int i = 0; i < 32; i++) walkFrames[i] = {i*64, 0, 64, 88};
    for (int i = 0; i < 3; i++) party[i] = 0;
}

mPlayer::~mPlayer() {
    freePlayer();
}

void mPlayer::freePlayer() {
    if (playerTexture != NULL) SDL_DestroyTexture(playerTexture);
    playerTexture = NULL;
    playerScoreList.freeHighScoreList();
}

bool mPlayer::loadPlayerData() {
    bool success = true;
    std::ifstream playerDatInStream("data/player.sav");

    playerScoreList.initHighScoreList();

    if (!playerDatInStream) {
        std::cout << "No save file detected! Default player config loaded instead!\n";
        success = false;
    } else {
        getline(playerDatInStream, name); // NAME
        playerDatInStream >> gender >> currentMap >> xCoords >> yCoords; // PLAYER LOCATION

        // PLAYER POKEMON DATA
        int partyIDs[3] = {0, 0, 0};
        playerDatInStream >> partyIDs[0] >> partyIDs[1] >> partyIDs[2];

        // PLAYER HIGH SCORE DATA
        int highScores[5] = {0, 0, 0, 0, 0};
        playerDatInStream >> currentHighScore;
        for (int i = 0; i < 5; i++) playerDatInStream >> highScores[i];

        // REJECT CORRUPTED SAVES INSTEAD OF INDEXING OUT OF BOUNDS WITH THEIR VALUES
        bool valid = !playerDatInStream.fail() && (gender == 0 || gender == 1)
            && currentMap >= 0 && currentMap < MAP_COUNT && xCoords >= 0 && yCoords >= 0 && currentHighScore >= 0;
        for (int id : partyIDs) valid = valid && id >= 0 && id < psize;
        for (int score : highScores) valid = valid && score >= 0;

        if (valid) {
            for (int i = 0; i < 3; i++) party[i] = partyIDs[i];
            playerScoreList.loadHighScoreList(highScores);
        } else {
            std::cout << "Save file is corrupted! Default player config loaded instead!\n";
            gender = 0;
            name = "Player";
            currentMap = 1;
            xCoords = 20, yCoords = 10;
            currentHighScore = 0;
            success = false;
        }
    }

    return success;
}

bool mPlayer::savePlayerData() {
    using namespace std;
    bool success = true;
    ofstream playerDatOutStream("data/player.sav");
    if (playerDatOutStream) {
        playerDatOutStream << name << endl;
        playerDatOutStream << gender << endl << currentMap << endl << xCoords << " " << yCoords << endl;
        playerDatOutStream << party[0].data - pokemonData << " " << party[1].data - pokemonData << " " << party[2].data - pokemonData << endl;
        playerDatOutStream << currentHighScore << endl;
        playerScoreList.saveHighScoreList(playerDatOutStream);
    } else {
        success = false;
    }
    return success;
}

void mPlayer::resetPlayerData() {
    gender = 0; // -1 = UNDEFINED, 0 = MALE, 1 = FEMALE
    name = "Player";
    currentMap = 1; // 0 = G2 EXTERIOR, 1 = E3 EXTERIOR, 2 = E3 INTERIOR, 3 = G2 INTERIOR, 4 = STUDENT BUTTON ROOM , 5 -> 10 = CHALLENGE ROOMS
    xCoords = 20, yCoords = 10;
    faceDirection = 0;
    currentHighScore = 0;
    playerScoreList.resetHighScoreList();
    initPlayerTexture();
    party[0] = 0;
    party[1] = 0;
    party[2] = 0;
}

int mPlayer::getGender() {
    return gender;
}

std::string mPlayer::getPlayerName() {
    return name;
}

int mPlayer::getCurrentMap() {
    return currentMap;
}

int mPlayer::getXCoords() {
    return xCoords;
}

int mPlayer::getYCoords() {
    return yCoords;
}

int mPlayer::getFacingDirection() {
    return faceDirection;
}

void mPlayer::initPlayerTexture() {
    if (playerTexture != NULL) SDL_DestroyTexture(playerTexture);
    playerTexture = NULL;
    if (gender == -1) {
        std::cout << "Player gender uninitiated!\n";
        return;
    }
    const char* path = gender == 0 ? "res/playersprite/malesprite.png" : "res/playersprite/femalesprite.png";
    SDL_Surface* tempSurface = IMG_Load(path);
    if (tempSurface == NULL) {
        std::cerr << "Failed to load " << path << ": " << IMG_GetError() << '\n';
        return;
    }
    SDL_SetColorKey(tempSurface, SDL_TRUE, SDL_MapRGB(tempSurface->format, 0, 255, 255));
    playerTexture = SDL_CreateTextureFromSurface(RenderWindow::renderer, tempSurface);
    SDL_FreeSurface(tempSurface);
}

void mPlayer::setPlayerCoords(int x, int y, int mapID) {
    xCoords = x;
    yCoords = y;
    currentMap = mapID;
}

void mPlayer::changeFacingDirect(int direct) {
    faceDirection = direct;
}

void mPlayer::setPlayerGender(int _gender) {
    if (_gender == 0) {
        name = "Ruby";
        gender = 0;
    } else if (_gender == 1) {
        name = "Sapphire";
        gender = 1;
    }
    initPlayerTexture();
}

void mPlayer::renderStandingPlayer() {
    switch (faceDirection) {
        case 0:
            SDL_RenderCopy(RenderWindow::renderer, playerTexture, &walkFrames[0], &screenCenter);
            break;
        case 1:
            SDL_RenderCopy(RenderWindow::renderer, playerTexture, &walkFrames[4], &screenCenter);
            break;
        case 2:
            SDL_RenderCopy(RenderWindow::renderer, playerTexture, &walkFrames[8], &screenCenter);
            break;
        case 3:
            SDL_RenderCopy(RenderWindow::renderer, playerTexture, &walkFrames[12], &screenCenter);
            break;
        default:
            SDL_RenderCopy(RenderWindow::renderer, playerTexture, &walkFrames[0], &screenCenter);
            break;
    }
}

void mPlayer::renderMovingPlayer() {
    moveFrame++;
    if (moveFrame > 60) moveFrame = 1;
    if (moveFrame % 10 == 0) walkFrame++;
    if (walkFrame > 3) walkFrame = 0;
    switch (faceDirection) {
        case 0:
            SDL_RenderCopy(RenderWindow::renderer, playerTexture, &walkFrames[0+walkFrame], &screenCenter);
            break;
        case 1:
            SDL_RenderCopy(RenderWindow::renderer, playerTexture, &walkFrames[4+walkFrame], &screenCenter);
            break;
        case 2:
            SDL_RenderCopy(RenderWindow::renderer, playerTexture, &walkFrames[8+walkFrame], &screenCenter);
            break;
        case 3:
            SDL_RenderCopy(RenderWindow::renderer, playerTexture, &walkFrames[12+walkFrame], &screenCenter);
            break;
        default:
            SDL_RenderCopy(RenderWindow::renderer, playerTexture, &walkFrames[0], &screenCenter);
            break;
    }
}

void mPlayer::renderRunningPlayer() {
    moveFrame++;
    if (moveFrame > 60) moveFrame = 1;
    if (moveFrame % 5 == 0) walkFrame++;
    if (walkFrame > 3) walkFrame = 0;
    switch (faceDirection) {
        case 0:
            SDL_RenderCopy(RenderWindow::renderer, playerTexture, &walkFrames[16+walkFrame], &screenCenter);
            break;
        case 1:
            SDL_RenderCopy(RenderWindow::renderer, playerTexture, &walkFrames[20+walkFrame], &screenCenter);
            break;
        case 2:
            SDL_RenderCopy(RenderWindow::renderer, playerTexture, &walkFrames[24+walkFrame], &screenCenter);
            break;
        case 3:
            SDL_RenderCopy(RenderWindow::renderer, playerTexture, &walkFrames[28+walkFrame], &screenCenter);
            break;
        default:
            SDL_RenderCopy(RenderWindow::renderer, playerTexture, &walkFrames[0], &screenCenter);
            break;
    }
}

// HIGH SCORE SYSTEM

void swap(int &a, int &b) {
    int c;
    c = a;
    a = b;
    b = c;
}

HighScoreList::HighScoreList() {
    head = NULL;
    highScoreScreenText = NULL;
    highScoreScreenRect = {0, 0, 0, 0};
}

HighScoreList::~HighScoreList() {}

void HighScoreList::initHighScoreList() {
    freeHighScoreList();
    head = new HighScoreListNode(0);
    HighScoreListNode* iterNode = head;
    for (int i = 0; i < 4; i++) {
        HighScoreListNode* nextNode = new HighScoreListNode(0);
        iterNode->nextHighScore = nextNode;
        iterNode = nextNode;
    }

    highScoreScreenText = IMG_LoadTexture(RenderWindow::renderer, "res/battleassets/pokemon_sel_screen.png");
    highScoreScreenRect = {91, 152, 650, 400};

    for (int i = 0; i < 5; i++) highScoreTexts[i].createFont("res/font/gamefont.ttf", 28);

    backButton.initMB("res/otherassets/backButton.png", 592, 473);
}

void HighScoreList::freeHighScoreList() {
    HighScoreListNode* p = head;
    while (p != NULL) {
        HighScoreListNode* p1 = p;
        p = p->nextHighScore;
        delete p1;
    }
    head = NULL;

    if (highScoreScreenText != NULL) SDL_DestroyTexture(highScoreScreenText);
    highScoreScreenText = NULL;
    for (int i = 0; i < 5; i++) highScoreTexts[i].freeText();
    backButton.freeButton();
}

void HighScoreList::loadHighScoreList(int highScoreList[]) {
    HighScoreListNode* iterNode = head;
    for (int i = 0; i < 5; i++) {
        iterNode->score = highScoreList[i];
        iterNode = iterNode->nextHighScore;
    }
}

void HighScoreList::printHighScoreList() {
    HighScoreListNode* iterNode = head;
    for (int i = 0; i < 5; i++) {
        std::cout << iterNode->score << std::endl;
        iterNode = iterNode->nextHighScore;
    }
}

void HighScoreList::saveHighScoreList(std::ofstream& saveStream) {
    HighScoreListNode* iterNode = head;
    for (int i = 0; i < 5; i++) {
        saveStream << iterNode->score << " ";
        iterNode = iterNode->nextHighScore;
    }
    saveStream << std::endl;
}

void HighScoreList::updateHighScoreList(int newHighScore) {
    HighScoreListNode* iterNode = head;
    HighScoreListNode* lastElement = NULL;
    bool insert = false;

    for (int i = 0; i < 5; i++) {
        if (newHighScore > iterNode->score && insert == false) {
            insert = true;
        }
        lastElement = iterNode;
        iterNode = iterNode->nextHighScore;
    }

    if (insert == true) {
        lastElement->score = newHighScore;

        HighScoreListNode* p1 = head;
        HighScoreListNode* p2 = NULL;
        while (p1 != NULL) {
            p2 = p1->nextHighScore;
            while (p2 != NULL) {
                if (p2->score > p1->score) {
                    swap(p1->score, p2->score);
                }
                p2 = p2->nextHighScore;
            }
            p1 = p1->nextHighScore;
        }
    } 
}

void HighScoreList::resetHighScoreList() {
    HighScoreListNode* iterNode = head;
    for (int i = 0; i < 5; i++) {
        iterNode->score = 0;
        iterNode = iterNode->nextHighScore;
    }
}

void HighScoreList::drawHighScoreScreen() {
    SDL_RenderCopy(RenderWindow::renderer, highScoreScreenText, NULL, &highScoreScreenRect);

    HighScoreListNode* iterNode = head;
    for (int i = 0; i < 5; i++) {
        highScoreTexts[i].textInit(RenderWindow::renderer, (to_string(i+1) + ". " + to_string(iterNode->score)).c_str(), {0,0,0});
        iterNode = iterNode->nextHighScore;
    }

    highScoreTexts[0].display(155, 217, RenderWindow::renderer);
    highScoreTexts[1].display(155, 270, RenderWindow::renderer);
    highScoreTexts[2].display(155, 323, RenderWindow::renderer);
    highScoreTexts[3].display(155, 376, RenderWindow::renderer);
    highScoreTexts[4].display(155, 429, RenderWindow::renderer);

    backButton.drawButton();
}