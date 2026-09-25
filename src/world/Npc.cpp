#include "world/Npc.h"
#include "core/RenderWindow.h"
#include "ui/OtherGraphics.h"
#include "Globals.h"
#include <iostream>

NPC::NPC() {
    npcXCoords = 0, npcYCoords = 0, faceDirection = 0;
    isTrainer = false, hasBattled = 2;
    npcTexture = NULL;
    dialogueCursor = 0;
}

NPC::~NPC() {
    if (npcTexture != NULL) SDL_DestroyTexture(npcTexture);
    npcTexture = NULL;
}

void NPC::initNPC(int x, int y, int face, const char* texturePath, bool canBattle) {
    npcXCoords = x, npcYCoords = y, faceDirection = face, isTrainer = canBattle;

    if (isTrainer) hasBattled = 0;

    if (npcTexture != NULL) SDL_DestroyTexture(npcTexture);
    npcTexture = NULL;
    SDL_Surface* tempSurface = IMG_Load(texturePath);
    if (tempSurface == NULL) {
        std::cerr << "Failed to load " << texturePath << ": " << IMG_GetError() << '\n';
    } else {
        SDL_SetColorKey(tempSurface, SDL_TRUE, SDL_MapRGB(tempSurface->format, 0, 255, 255));
        npcTexture = SDL_CreateTextureFromSurface(RenderWindow::renderer, tempSurface);
        SDL_FreeSurface(tempSurface);
    }
    
    for (int i = 0; i < 16; i++) {
        walkFrames[i] = {i*64, 0, 64, 88};
    }
}

void NPC::drawNPC(int camX, int camY) {
    SDL_Rect destBox = {npcXCoords*64 - camX, npcYCoords*64 - camY - 24, 64, 88};
    switch (faceDirection) {
    case 0:
        SDL_RenderCopy(RenderWindow::renderer, npcTexture, &walkFrames[0], &destBox);
        break;
    case 1:
        SDL_RenderCopy(RenderWindow::renderer, npcTexture, &walkFrames[4], &destBox);
        break;
    case 2:
        SDL_RenderCopy(RenderWindow::renderer, npcTexture, &walkFrames[8], &destBox);
        break;
    case 3:
        SDL_RenderCopy(RenderWindow::renderer, npcTexture, &walkFrames[12], &destBox);
        break;
    default:
        SDL_RenderCopy(RenderWindow::renderer, npcTexture, &walkFrames[0], &destBox);
        break;
    }
}

bool NPC::talkNPC(int playerFace) {
    switch (playerFace) {
    case 0:
        faceDirection = 2;
        break;
    case 1:
        faceDirection = 3;
        break;
    case 2:
        faceDirection = 0;
        break;
    case 3:
        faceDirection = 1;
        break;
    default:
        break;
    }

    if (isTrainer == true)
    {
        if (hasBattled == 0) {
            if (dialogueCursor < preBattleTexts.size()) {
                dialogueCursor++;
                return true;
            } else {
                dialogueCursor = 0;
                beginMapToBattleTransition = true;
                return false;
            }
        } else {
            if (dialogueCursor < dialogueTexts.size()) {
                dialogueCursor++;
                return true;
            } else {
                dialogueCursor = 0;
                return false;
            }
        }
    } 
    else
    {
        if (dialogueCursor < dialogueTexts.size()) {
            dialogueCursor++;
            return true;
        } else {
            dialogueCursor = 0;
            return false;
        }
    }

    
    // else {
        
    //     if (isTrainer == true and hasBattled == 0) {
    //         beginMapToBattleTransition = true;
    //     }
        
    // }
}

void NPC::initDialogue(std::string nextSentence) {
    dialogueTexts.push_back(nextSentence);
}

void NPC::initPreBattleDialogue(std::string nextSentence) {
    preBattleTexts.push_back(nextSentence);
}

std::string NPC::getCurrentSentence()
{
    const std::vector<std::string>& texts = (isTrainer == true and hasBattled == 0) ? preBattleTexts : dialogueTexts;
    if (dialogueCursor == 0 or dialogueCursor > texts.size()) return "";
    return texts[dialogueCursor - 1];
}

int NPC::getCurrentSentenceID() {
    return dialogueCursor;
}