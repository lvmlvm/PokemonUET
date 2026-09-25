#include <iostream>
#include <cmath>

#include "scenes/BattleScreen.h"
#include "core/RenderWindow.h"
#include "Globals.h"
#include "core/Rng.h"

// BATTLE RENDERING

static SDL_Rect halfPlayerPokeRect = {0, 0, 96, 96};
static SDL_Rect halfOppoPokeRect = {96, 0, 96, 96};

static int currentPlayerFrame = 0;
static unsigned int BattleSen = 0;

static int playerFaintedPokemons = 0, opponentFaintedPokemons = 0;
static int playerBeforeAttackHP = 0, opponentBeforeAttackHP = 0,
            playerAfterAttackHP = 0, opponentAfterAttackHP = 0;

static double pCirX = 0, oCirX = 0, playerPokemonMoveAnim = 0, opponentPokemonMoveAnim = 0, PokemonTransparency = 0;

BattleScreen::BattleScreen() {
    battlePlayer = NULL;
    battleOpponent = NULL;
    currentPlayerPokemon = NULL;
    currentOpponentPokemon = NULL;
    battleBackground = battleCircle = grayInputBox = NULL;
    playerTexture = opponentTexture = NULL;
    playerHPBar = oppoHPBar = HPColor = NULL;
    playerPokeTexture = opponentPokeTexture = NULL;
    effectiveSFX = notEffectiveSFX = superEffectiveSFX = pokemonFainted = NULL;
}

const std::string& BattleScreen::actionAt(unsigned int i) const {
    static const std::string none;
    return i < turnActionQueue.size() ? turnActionQueue[i] : none;
}

const std::string& BattleScreen::dialogueAt(unsigned int i) const {
    static const std::string none;
    return i < battleDialogues.size() ? battleDialogues[i] : none;
}

BattleScreen::~BattleScreen() {
	freeBattleScreen();
}

void BattleScreen::initBattleScreen(mPlayer* player, Trainer* opponent) {
    battlePlayer = player;
    battleOpponent = opponent;

    currentPlayerPokemon = &(player->party[0]);
    currentOpponentPokemon = &(opponent->party[0]);

    bd_Text.createFont("res/font/gamefont.ttf", 42);
    playerPokeName.createFont("res/font/gamefont.ttf", 28);
    oppoPokeName.createFont("res/font/gamefont.ttf", 28);

    // LOAD THE MAIN BATTLE ASSETS
	battleBackground = IMG_LoadTexture(RenderWindow::renderer, "res/battleassets/battlebackground0.png");
	battleCircle = IMG_LoadTexture(RenderWindow::renderer, "res/battleassets/battlecircle0.png");
	grayInputBox = IMG_LoadTexture(RenderWindow::renderer, "res/battleassets/graybox.png");
	playerCircle = {832, 450, 320, 100};
	opponentCircle = {-320, 200, 320, 100};
	grayBoxRect = {0, 704, 832, 204};

    // HP BAR TEXTURES;
    playerHPBar = IMG_LoadTexture(RenderWindow::renderer, "res/battleassets/pHPBar.png");
    oppoHPBar = IMG_LoadTexture(RenderWindow::renderer, "res/battleassets/oHPBar.png");
    HPColor = IMG_LoadTexture(RenderWindow::renderer, "res/battleassets/HPColor.png");
    playerHPRect = {548, 388, 284, 95};
    oppoHPRect = {0, 66, 273, 78};
    currPlayHP = {708, 443, int(ceil(106.0*(double(currentPlayerPokemon->c_hp)/double(currentPlayerPokemon->data->hp)))), 7};
    currOppoHP = {112, 119, int(ceil(107.0*(double(currentOpponentPokemon->c_hp)/double(currentOpponentPokemon->data->hp)))), 7};

    // CHARACTER SPRITE BOXES
    playerSpriteBox = {832, 260, 240, 240};
    opponentSpriteBox = {-320, 20, 240, 240};

    // POKEMON SPRITE BOXES
    playerPokeRect = {-240, 300, 240, 240};
    oppoPokeRect = {832, 20, 240, 240};

    // PLAYER SPRITE HANDLING
    for (int i = 0; i < 4; i++) playerTextureFrames[i] = {i*80, 0, 80, 80};
    if (player->getGender() == 0) playerTexture = IMG_LoadTexture(RenderWindow::renderer, "res/battleassets/malefrontsprite.png");
    else if (player->getGender() == 1) playerTexture = IMG_LoadTexture(RenderWindow::renderer, "res/battleassets/femalefrontsprite.png");

    // OPPONENT SPRITE HANDLING
    opponentTexture = IMG_LoadTexture(RenderWindow::renderer, opponent->battleSpritePath.c_str());

    // BATTLE DIALOGUE (FIRST 3 SENTENCES ONLY)
    battleDialogues.clear();
    std::string sen1 = "You are challenged by " + opponent->name + "!";
    battleDialogues.push_back(sen1);
    std::string sen2 = opponent->name + " sent out " + currentOpponentPokemon->data->name + "!";
    battleDialogues.push_back(sen2);
    std::string sen3 = player->getPlayerName() + " sent out " + currentPlayerPokemon->data->name + "!";
    battleDialogues.push_back(sen3);
    turnActionQueue.push_back("BATTLE_START");

    // STARTING POKEMON TEXTURE
    std::string imgPlayerPath = "res/pokemonassets/" + battlePlayer->party[0].data->name + ".png";
    playerPokeTexture = IMG_LoadTexture(RenderWindow::renderer, imgPlayerPath.c_str());
    std::string imgOppoPath = "res/pokemonassets/" + battleOpponent->party[0].data->name + ".png";
    opponentPokeTexture = IMG_LoadTexture(RenderWindow::renderer, imgOppoPath.c_str());

    // INIT THE FIGHT SCREEN BUTTONS
    fightButton.initBSB("res/battleassets/fightbutton.png", 230, 527, 371, 150, 556, 224);
    pokemonButton.initBSB("res/battleassets/pokemonbutton.png", 23, 559, 188, 86, 282, 130);
    retireButton.initBSB("res/battleassets/retirebutton.png", 621, 559, 188, 86, 282, 130);

    // INIT THE MOVE SCREEN BUTTONS
    moveButtons[0].initBSB("res/battleassets/moveButton.png", 33, 519, 278, 72, 652, 167, true);
    moveButtons[1].initBSB("res/battleassets/moveButton.png", 344, 519, 278, 72, 652, 167, true);
    moveButtons[2].initBSB("res/battleassets/moveButton.png", 33, 611, 278, 72, 652, 167, true);
    moveButtons[3].initBSB("res/battleassets/moveButton.png", 344, 611, 278, 72, 652, 167, true);
    backButton.initBSB("res/battleassets/backbutton.png", 659, 620, 154, 60, 362, 139);

    // INIT THE SOUND EFFECTS
    effectiveSFX = Mix_LoadWAV("res/sfx/normal_effective.wav");
    notEffectiveSFX = Mix_LoadWAV("res/sfx/not_effective.wav");
    superEffectiveSFX = Mix_LoadWAV("res/sfx/super_effective.wav");
    pokemonFainted = Mix_LoadWAV("res/sfx/poke_fainted.wav");

    // INIT THE POKEMON SELECTION SCREEN
    selScreen.initSelectionScreen(&mainPlayer);
}

void BattleScreen::freeBattleScreen() {
    // ALL TEXTURES
    SDL_Texture** textures[] = {&battleBackground, &battleCircle, &grayInputBox, &playerTexture, &opponentTexture,
                                &playerPokeTexture, &opponentPokeTexture, &playerHPBar, &oppoHPBar, &HPColor};
    for (SDL_Texture** texture : textures) {
        if (*texture != NULL) SDL_DestroyTexture(*texture);
        *texture = NULL;
    }

    bd_Text.freeText();
    playerPokeName.freeText();
    oppoPokeName.freeText();

    playerCircle = {832, 450, 320, 100};
	opponentCircle = {-320, 200, 320, 100};
	grayBoxRect = {0, 704, 832, 204};
    playerSpriteBox = {872, 260, 240, 240};
    opponentSpriteBox = {-320, 20, 240, 240};

    // ON SCREEN POKEMANS
    playerPokeRect = {-240, 300, 240, 240};
    oppoPokeRect = {832, 20, 240, 240};

    // HP Bars
    playerHPRect = {548, 388, 284, 95};
    oppoHPRect = {0, 66, 273, 78};
    currPlayHP = {708, 443, 106, 7};
    currOppoHP = {112, 119, 107, 7};

    // Free the buttons
    fightButton.freeButton();
    pokemonButton.freeButton();
    retireButton.freeButton();

    for (int i = 0; i < 4; i++) moveButtons[i].freeButton();
    backButton.freeButton();

    selScreen.freeSelectionScreen();

    battleDialogues.clear();
    turnActionQueue.clear();

    if (effectiveSFX != NULL) Mix_FreeChunk(effectiveSFX);
    if (notEffectiveSFX != NULL) Mix_FreeChunk(notEffectiveSFX);
    if (superEffectiveSFX != NULL) Mix_FreeChunk(superEffectiveSFX);
    if (pokemonFainted != NULL) Mix_FreeChunk(pokemonFainted);
    effectiveSFX=NULL;
    notEffectiveSFX=NULL;
    superEffectiveSFX=NULL;
    pokemonFainted=NULL;

	pCirX = 0;
	oCirX = 0;
    playerPokemonMoveAnim = 0;
    opponentPokemonMoveAnim = 0;
    PokemonTransparency = 0;

    playerFaintedPokemons = 0;
    opponentFaintedPokemons = 0;
    playerBeforeAttackHP = 0, opponentBeforeAttackHP = 0, playerAfterAttackHP = 0, opponentAfterAttackHP = 0;

    inAnim0 = true;
    showPHPBar = false, showOHPBar = false;
    fightScreen = false, moveScreen = false, inSelectionScreen = false;
    startingBattle = true;

    BattleSen = 0;
    currentPlayerFrame = 0;
}

void BattleScreen::drawBattleScreen(bool fMtB, bool fBtM) {
    // RENDER THE BATTLE BACKGROUND
	SDL_RenderCopy(RenderWindow::renderer, battleBackground, NULL, NULL);

    // ANIMATING THE PLAYER AND OPPONENT SPRITES
	if (fMtB == false) {

        // I WAS BRAINDEAD WHILE CODING THIS PARTICULAR ANIMATION :(
        // HINH DUNG CAI PHUONG TRINH BAC 2 O DUOI LA 1 CAI PHEP ANH XA, OCIRX LA X
        // X CHAY TU 0 DEN 100. CAI PHUONG TRINH BAC 2 DAY NHU LA PHUONG TRINH CHUYEN DONG CUA MAY CAI SPRITE TREN MAN HINH THOI
        // T DA THU REVERT LAI THANH OCIRX = 0 -> 100. NHMA IT BROKE THE ANIMATION SO BARE WITH IT PLZ
        
        if (-0.1276*oCirX*oCirX+20.58*oCirX-320 < 462) {
            opponentSpriteBox.x = int(-0.1276*oCirX*oCirX+20.58*oCirX-300);
			opponentCircle.x = int(-0.1276*oCirX*oCirX+20.58*oCirX-320);
			oCirX++;
		}
		SDL_RenderCopy(RenderWindow::renderer, battleCircle, NULL, &opponentCircle);
        SDL_RenderCopy(RenderWindow::renderer, opponentTexture, NULL, &opponentSpriteBox);            
		if (-0.1276*oCirX*oCirX+20.58*oCirX-320 < 462) {
			playerCircle.x = int(0.1244*pCirX*pCirX-20.26*pCirX+832);
            playerSpriteBox.x = int(0.1244*pCirX*pCirX-20.26*pCirX+862);
            pCirX++;
            if (-0.1276*oCirX*oCirX+20.58*oCirX-320 > 430) {
                inAnim0 = false;
            }
		}
		SDL_RenderCopy(RenderWindow::renderer, battleCircle, NULL, &playerCircle);
        SDL_RenderCopy(RenderWindow::renderer, playerTexture, &playerTextureFrames[currentPlayerFrame], &playerSpriteBox);  
	}

    // RENDER THE 2 SIDE'S POKEMONS
    SDL_RenderCopy(RenderWindow::renderer, opponentPokeTexture, &halfOppoPokeRect, &oppoPokeRect);
    SDL_RenderCopy(RenderWindow::renderer, playerPokeTexture, &halfPlayerPokeRect, &playerPokeRect);

    // RENDER THE GRAY BOX CONTAINING THE DIALOGUE AND THE TEXT
	if (grayBoxRect.y > 500) grayBoxRect.y -= 4;
	SDL_RenderCopy(RenderWindow::renderer, grayInputBox, NULL, &grayBoxRect);

    // the first battle sentence
    if (fMtB == false and BattleSen == 0 and startingBattle == true) {
        bd_Text.textInit(RenderWindow::renderer, dialogueAt(BattleSen).c_str(), {255, 255, 255}, 800);
        bd_Text.display(32, 530, RenderWindow::renderer);
    }

    // opponent sending out the pokemons
    if (BattleSen == 1 and startingBattle == true) {
        bd_Text.textInit(RenderWindow::renderer, dialogueAt(BattleSen).c_str(), {255, 255, 255}, 800);
        bd_Text.display(32, 530, RenderWindow::renderer);
        if (opponentSpriteBox.x < 860) {
            opponentSpriteBox.x += 8;
        }

        if (opponentSpriteBox.x > 850) {
            if (oppoPokeRect.x > 500) {
                oppoPokeRect.x -= 8;
                if (oppoPokeRect.x < 510) {
                    inAnim0 = false;
                    showOHPBar = true;
                }
            }
        }
    }

    // player sending out the pokemons
    if (BattleSen == 2 and startingBattle == true) {
        bd_Text.textInit(RenderWindow::renderer, dialogueAt(BattleSen).c_str(), {255, 255, 255}, 800);
        bd_Text.display(32, 530, RenderWindow::renderer);
        if (playerSpriteBox.x > -240) {
            playerSpriteBox.x -= 8;
            
            if (playerSpriteBox.x == 70) currentPlayerFrame++;
            if (playerSpriteBox.x == 22) currentPlayerFrame++;
            if (playerSpriteBox.x == -26) currentPlayerFrame++;
        }

        if (playerSpriteBox.x < -230) {
            if (playerPokeRect.x < 90) {
                playerPokeRect.x += 8;
                if (playerPokeRect.x > 80) {
                    inAnim0 = false;
                    showPHPBar = true;
                }
            }
        }
    }

    // SHOW THE OPPONENT'S HP BAR
    if (showOHPBar == true) {
        SDL_RenderCopy(RenderWindow::renderer, oppoHPBar, NULL, &oppoHPRect);
        SDL_RenderCopy(RenderWindow::renderer, HPColor, NULL, &currOppoHP);
        oppoPokeName.textInit(RenderWindow::renderer, (currentOpponentPokemon->data->name).c_str(), {0, 0, 0});
        oppoPokeName.display(4, 83, RenderWindow::renderer);
    }

    // SHOW THE PLAYER'S HP BAR
    if (showPHPBar == true) {
        SDL_RenderCopy(RenderWindow::renderer, playerHPBar, NULL, &playerHPRect);
        SDL_RenderCopy(RenderWindow::renderer, HPColor, NULL, &currPlayHP);
        playerPokeName.textInit(RenderWindow::renderer, (currentPlayerPokemon->data->name).c_str(), {0, 0, 0});
        playerPokeName.display(585, 407, RenderWindow::renderer);
    }

    // DISPLAY THE FIGHT SCREEN
    if (fightScreen == true) {
        fightButton.drawButton();
        pokemonButton.drawButton();
        retireButton.drawButton();
    }

    // DISPLAY THE MOVE SCREEN
    if (moveScreen == true) {
        for (int i = 0; i < 4; i++) moveButtons[i].drawButton(true);
        backButton.drawButton();
    }

    // HANDLING MOVE ANIMATION AND FLAGS AND BATTLE DIALOGUES 
    if (startingBattle == false and fightScreen == false and moveScreen == false) {
        bd_Text.textInit(RenderWindow::renderer, dialogueAt(BattleSen).c_str(), {255, 255, 255}, 800);
        bd_Text.display(32, 530, RenderWindow::renderer);

        if (actionAt(BattleSen) == "PLAYER_USE_MOVE") // ANIMATE THE PLAYER USING A MOVE
        {
            if (playerPokemonMoveAnim < 21 and inAnim0 == true) {
                playerPokeRect.x = -(playerPokemonMoveAnim*playerPokemonMoveAnim) + 20*playerPokemonMoveAnim + 90;
                playerPokemonMoveAnim++;
            } else if (playerPokemonMoveAnim == 21 and inAnim0 == true) {
                if (PokemonTransparency == 0) {
                    if (BattleSen + 1 < turnActionQueue.size()) {
                        if (actionAt(BattleSen+1) == "MOVE_NOEFFECT") {
                            playerPokemonMoveAnim = 0;
                            PokemonTransparency = 0;
                            inAnim0 = false;
                        } else if (actionAt(BattleSen+1) == "MOVE_NOT_EFFECTIVE") {
                            Mix_PlayChannel(-1, notEffectiveSFX, 0);
                        } else if (actionAt(BattleSen+1) == "MOVE_SUPER_EFFECTIVE") {
                            Mix_PlayChannel(-1, superEffectiveSFX, 0);
                        } else Mix_PlayChannel(-1, effectiveSFX, 0);
                    } else Mix_PlayChannel(-1, effectiveSFX, 0);
                }

                if (PokemonTransparency < 1170 and inAnim0 == true) {
                    SDL_SetTextureAlphaMod(opponentPokeTexture, int(255*(abs(sin(PokemonTransparency*3.141592/180.0)))));
                    PokemonTransparency += 130;
                } else if (PokemonTransparency >= 1170 and inAnim0 == true) {
                    SDL_SetTextureAlphaMod(opponentPokeTexture, int(255*(abs(sin(PokemonTransparency*3.141592/180.0)))));
                    playerPokemonMoveAnim = 0;
                    PokemonTransparency = 0;
                    currOppoHP.w = int(ceil(107.0*(double(currentOpponentPokemon->c_hp)/double(currentOpponentPokemon->data->hp))));
                    inAnim0 = false;
                }
            }
        }

        // ANIMATE THE OPPONENT USING A MOVE, HANDLING THE BATTLE SCRIPTS
        else if (actionAt(BattleSen) == "OPPONENT_USE_MOVE") 
        {
            if (opponentPokemonMoveAnim < 21 and inAnim0 == true) {
                oppoPokeRect.x = (opponentPokemonMoveAnim*opponentPokemonMoveAnim) - 20*opponentPokemonMoveAnim + 500;
                opponentPokemonMoveAnim++;
            } else if (opponentPokemonMoveAnim == 21 and inAnim0 == true) {
                if (PokemonTransparency == 0) {
                    if (BattleSen + 1 < turnActionQueue.size()) {
                        if (actionAt(BattleSen+1) == "MOVE_NOEFFECT") {
                            opponentPokemonMoveAnim = 0;
                            PokemonTransparency = 0;
                            inAnim0 = false;
                        } else if (actionAt(BattleSen+1) == "MOVE_NOT_EFFECTIVE") {
                            Mix_PlayChannel(-1, notEffectiveSFX, 0);
                        } else if (actionAt(BattleSen+1) == "MOVE_SUPER_EFFECTIVE") {
                            Mix_PlayChannel(-1, superEffectiveSFX, 0);
                        } else Mix_PlayChannel(-1, effectiveSFX, 0);
                    } else Mix_PlayChannel(-1, effectiveSFX, 0);
                }

                if (PokemonTransparency < 1170 and inAnim0 == true) {
                    SDL_SetTextureAlphaMod(playerPokeTexture, int(255*(abs(sin(PokemonTransparency*3.141592/180.0)))));
                    PokemonTransparency += 130;
                } else if (PokemonTransparency >= 1170 and inAnim0 == true) {
                    SDL_SetTextureAlphaMod(playerPokeTexture, int(255*(abs(sin(PokemonTransparency*3.141592/180.0)))));
                    opponentPokemonMoveAnim = 0;
                    PokemonTransparency = 0;
                    currPlayHP.w = int(ceil(106.0*(double(currentPlayerPokemon->c_hp)/double(currentPlayerPokemon->data->hp))));
                    inAnim0 = false;
                }
            }
        }

        else if (actionAt(BattleSen) == "OPPONENT_FAINT") 
        {
            if ((halfOppoPokeRect.h > 0 or oppoPokeRect.y < 260) and inAnim0 == true) {
                halfOppoPokeRect.h -= 10;
                oppoPokeRect.y += 24;
                oppoPokeRect.h -= 24;
            } else if (halfOppoPokeRect.h <= 0 and oppoPokeRect.y == 260 and inAnim0 == true) {
                Mix_PlayChannel(-1, pokemonFainted, 0);
                showOHPBar = false;
                oppoPokeRect = {832, 20, 240, 240};
                halfOppoPokeRect.h = 96;
                inAnim0 = false;
            }
        }

        else if (actionAt(BattleSen) == "OPPONENT_NEXT_POKEMON") {
            if (oppoPokeRect.x > 500) {
                if (oppoPokeRect.x == 832) {
                    SDL_DestroyTexture(opponentPokeTexture);
                    std::string imgOppoPath = "res/pokemonassets/" + battleOpponent->party[opponentFaintedPokemons].data->name + ".png";
                    opponentPokeTexture = IMG_LoadTexture(RenderWindow::renderer, imgOppoPath.c_str());
                }
                oppoPokeRect.x -= 8;
                if (oppoPokeRect.x < 510) {
                    currentOpponentPokemon = &(battleOpponent->party[opponentFaintedPokemons]);
                    inAnim0 = false;
                    showOHPBar = true;
                    currOppoHP.w = int(ceil(107.0*(double(currentOpponentPokemon->c_hp)/double(currentOpponentPokemon->data->hp))));
                }
            }
        }

        else if (actionAt(BattleSen) == "PLAYER_FAINT") 
        {
            if ((halfPlayerPokeRect.h > 0 or playerPokeRect.y < 540) and inAnim0 == true) {
                halfPlayerPokeRect.h -= 10;
                playerPokeRect.y += 24;
                playerPokeRect.h -= 24;
            } else if (halfPlayerPokeRect.h <= 0 and playerPokeRect.y == 540 and inAnim0 == true) {
                Mix_PlayChannel(-1, pokemonFainted, 0);
                showPHPBar = false;
                playerPokeRect = {-240, 300, 240, 240};
                halfPlayerPokeRect.h = 96;
                inAnim0 = false;
            }
        }

        else if (actionAt(BattleSen) == "FORCE_OPEN_PARTY")
        {
            inSelectionScreen = true;
            selScreen.updateSelectionScreen(battlePlayer);
            inAnim0 = false;
        }

        else if (actionAt(BattleSen) == "PLAYER_NEXT_POKEMON")
        {
            if (playerPokeRect.x < 90) {
                playerPokeRect.x += 8;
                if (playerPokeRect.x > 80) {
                    inAnim0 = false;
                    if (actionAt(BattleSen-1) == "PLAYER_WITHDREW_POKEMON") currPlayHP = {708, 443, int(ceil(106.0*(double(playerBeforeAttackHP)/double(currentPlayerPokemon->data->hp)))), 7};
                    else currPlayHP = {708, 443, int(ceil(106.0*(double(currentPlayerPokemon->c_hp)/double(currentPlayerPokemon->data->hp)))), 7};
                    showPHPBar = true;
                }
            }
        }

        else if (actionAt(BattleSen) == "PLAYER_WITHDREW_POKEMON")
        {
            if (playerPokeRect.x > -240) {
                playerPokeRect.x -= 8;
                if (playerPokeRect.x < -230) {
                    inAnim0 = false;
                    SDL_DestroyTexture(playerPokeTexture);
                    std::string imgPlayerPath = "res/pokemonassets/" + currentPlayerPokemon->data->name + ".png";
                    playerPokeTexture = IMG_LoadTexture(RenderWindow::renderer, imgPlayerPath.c_str());
                }
            }
        }

        else if (actionAt(BattleSen) == "PLAYER_DEFEATED")
        {
            battlePlayer->playerScoreList.updateHighScoreList(battlePlayer->getCurrentHighScore());
            battlePlayer->resetCurrentHighScore();
            playerMap->freeMap();
            playerMap->loadMap(gameMaps[1].c_str(), gameTileSets[1].c_str(), gameThemes[1].c_str(), themeRepeats[1], mapOverlays[1]);
            mainPlayer.setPlayerCoords(20, 10, 1);
            mainCamera.setCameraPos((mainPlayer.getXCoords() - 6) * 64, (mainPlayer.getYCoords() - 5) * 64);
            mainPlayer.setFacingDirection(0);
            inAnim0 = false;
        }

        else if (actionAt(BattleSen) == "END_BATTLE")
        {
            beginBattleToMapTransition = true;
        }

        else
        {
            inAnim0 = false;
        }
    }

    // DISPLAY THE SELECTION SCREEN
    if (inSelectionScreen == true) {
        selScreen.display();
    }
}

void BattleScreen::centralBattleProcess(SDL_Event* e) {
    // HANDLING WHEN THE X BUTTON IS PRESSED (USED MOSTLY FOR BATTLE DIALOGUES)
    if ((e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_x and e->key.repeat == 0) && inAnim0 == false && fightScreen == false && moveScreen == false && inSelectionScreen == false) {
        if (startingBattle == true) {
            if (BattleSen < 3) {
                inAnim0 = true;
                BattleSen++;
                Mix_PlayChannel(-1, aButton, 0);
                if (BattleSen == 3) {
                    fightScreen = true;
                    startingBattle = false;
                    BattleSen = 0;
                    battleDialogues.clear();
                    turnActionQueue.clear();
                }
            }
        } else {
            inAnim0 = true;
            BattleSen++;
            Mix_PlayChannel(-1, aButton, 0);
            if (BattleSen == battleDialogues.size()) {
                fightScreen = true;
                BattleSen = 0;
                battleDialogues.clear();
                turnActionQueue.clear();
            }
        }
        return;
    }

    // IN THE POKEMON SELECTION SCREEN
    if (inSelectionScreen == true) {
        // EXIT THE MENU WITH ESCAPE KEY
        if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_ESCAPE && actionAt(BattleSen) != "FORCE_OPEN_PARTY") {
            inSelectionScreen = false;
            pokemonButton.buttonHandler();
            return;
        }

        // EXIT THE MENU WITH BACK BUTTON
        selScreen.backButton.buttonHandler();
        if (selScreen.backButton.clickedOn == true && actionAt(BattleSen) != "FORCE_OPEN_PARTY") {
            selScreen.backButton.clickedOn = false;
            inSelectionScreen = false;
            return;
        }

        // HANDLE THE POKEMON SELECTIONS
        for (int i = 0; i < 3; i++) {
            selScreen.pokemonSelectionButton[i].buttonHandler();
            if (selScreen.pokemonSelectionButton[i].clickedOn) {
                selScreen.pokemonSelectionButton[i].clickedOn = false;
                pokemonButton.buttonHandler();
                localSwitchPokemonHandler(i);
                return;
            }
        }
    }    

    // HANDLING THE SELECT MOVE SCREEN
    if (moveScreen == true) {
        backButton.buttonHandler();
        if (backButton.clickedOn == true) {
            backButton.clickedOn = false;
            Mix_PlayChannel(-1, clickedOnSound, 0);
            fightScreen = true;
            moveScreen = false;
            return;
        }

        for (int i = 0; i < 4; i++) {
            moveButtons[i].buttonHandler();
            moveButtons[i].moveButtonHandler(i);
            if (moveButtons[i].clickedOn == true) {
                moveButtons[i].clickedOn = false;
                Mix_PlayChannel(-1, clickedOnSound, 0);
                localTurnHandler(i);
                moveScreen = false;
                return;
            }
        }
    }

    // HANDLING THE FIGHT SCREEN
    if (fightScreen == true && inSelectionScreen == false) {
        fightButton.buttonHandler();
        pokemonButton.buttonHandler();
        retireButton.buttonHandler();
        if (fightButton.clickedOn == true) {
            moveScreen = true;
            fightScreen = false;
            fightButton.clickedOn = false;
            Mix_PlayChannel(-1, clickedOnSound, 0);
            for (int i = 0; i < 4; i++) moveButtons[i].moveButtonHandler(i);
            return;
        } else if (pokemonButton.clickedOn == true) {
            pokemonButton.clickedOn = false;
            Mix_PlayChannel(-1, clickedOnSound, 0);
            selScreen.updateSelectionScreen(battlePlayer);
            inSelectionScreen = true;
            return;
        } else if (retireButton.clickedOn == true) {
            retireButton.clickedOn = false;
            battlePlayer->playerScoreList.updateHighScoreList(battlePlayer->getCurrentHighScore());
            battlePlayer->resetCurrentHighScore();
            playerMap->freeMap();
            playerMap->loadMap(gameMaps[1].c_str(), gameTileSets[1].c_str(), gameThemes[1].c_str(), themeRepeats[1], mapOverlays[1]);
            mainPlayer.setPlayerCoords(20, 10, 1);
            mainCamera.setCameraPos((mainPlayer.getXCoords() - 6) * 64, (mainPlayer.getYCoords() - 5) * 64);
            mainPlayer.setFacingDirection(0);
            Mix_PlayChannel(-1, clickedOnSound, 0);
            beginBattleToMapTransition = true;
            return;
        }
    }
}

void BattleScreen::localTurnHandler(int move) {
    bool isKO;
    if (currentPlayerPokemon->data->speed >= currentOpponentPokemon->data->speed) { // IF THE PLAYER IS FASTER OR HAS SAME SPEED AS OPPONENT
        isKO = useMove(move, *currentPlayerPokemon, *currentOpponentPokemon, false);
        if (isKO) {
            std::string newSentence = "The opposing " + currentOpponentPokemon->data->name + " fainted!";
            battleDialogues.push_back(newSentence);
            turnActionQueue.push_back("OPPONENT_FAINT");

            battlePlayer->updateCurrentHighScore(currentOpponentPokemon->data->hp);
            std::string newXPSentence = battlePlayer->getPlayerName() + " scored " + to_string(currentOpponentPokemon->data->hp) + " points!";
            battleDialogues.push_back(newXPSentence);
            turnActionQueue.push_back("PLAYER_GAIN_SCORE");

            opponentFaintedPokemons++;
            if (opponentFaintedPokemons < 3) {
                std::string newSentence = battleOpponent->name + " sent out " + battleOpponent->party[opponentFaintedPokemons].data->name + "!";
                battleDialogues.push_back(newSentence);
                turnActionQueue.push_back("OPPONENT_NEXT_POKEMON");
            } else {
                std::string newSentence = battlePlayer->getPlayerName() + " defeated " + battleOpponent->name + "!";
                battleDialogues.push_back(newSentence);
                turnActionQueue.push_back("OPPONENT_DEFEATED");
                battleDialogues.push_back(" ");
                turnActionQueue.push_back("END_BATTLE");
            }
        }
        if (!isKO) { // IF THE OPPONENT SURVIVES THE PLAYER'S ATTACK
            isKO = useMove(computerChooseMove(*currentOpponentPokemon, *currentPlayerPokemon), *currentOpponentPokemon, *currentPlayerPokemon, true);
            if (isKO) {
                std::string newSentence = currentPlayerPokemon->data->name + " fainted!";
                battleDialogues.push_back(newSentence);
                turnActionQueue.push_back("PLAYER_FAINT");
                playerFaintedPokemons++;
                if (playerFaintedPokemons < 3) {
                    battleDialogues.push_back("Awaiting " + battlePlayer->getPlayerName() + "'s next Pokemon...");
                    turnActionQueue.push_back("FORCE_OPEN_PARTY");
                } else {
                    battleDialogues.push_back(battlePlayer->getPlayerName() + " lost to " + battleOpponent->name + "!");
                    turnActionQueue.push_back("PLAYER_DEFEATED");
                    battleDialogues.push_back(" ");
                    turnActionQueue.push_back("END_BATTLE");
                }
            }
        }
    } else { // THE OPPONENT IS FASTER THAN THE PLAYER
        isKO = useMove(computerChooseMove(*currentOpponentPokemon, *currentPlayerPokemon), *currentOpponentPokemon, *currentPlayerPokemon, true);
        if (isKO) {
            std::string newSentence = currentPlayerPokemon->data->name + " fainted!";
            battleDialogues.push_back(newSentence);
            turnActionQueue.push_back("PLAYER_FAINT");
            playerFaintedPokemons++;
            if (playerFaintedPokemons < 3) {
                battleDialogues.push_back("Awaiting " + battlePlayer->getPlayerName() + "'s next Pokemon...");
                turnActionQueue.push_back("FORCE_OPEN_PARTY");
            } else {
                battleDialogues.push_back(battlePlayer->getPlayerName() + " lost to " + battleOpponent->name + "!");
                turnActionQueue.push_back("PLAYER_DEFEATED");
                battleDialogues.push_back(" ");
                turnActionQueue.push_back("END_BATTLE");
            }
        } // IF THE PLAYER SURVIVES THE OPPONENT'S ATTACK
        if (!isKO) {
            isKO = useMove(move, *currentPlayerPokemon, *currentOpponentPokemon, false);
            if (isKO) {
                std::string newSentence = "The opposing " + currentOpponentPokemon->data->name + " fainted!";
                battleDialogues.push_back(newSentence);
                turnActionQueue.push_back("OPPONENT_FAINT");

                battlePlayer->updateCurrentHighScore(currentOpponentPokemon->data->hp);
                std::string newXPSentence = battlePlayer->getPlayerName() + " scored " + to_string(currentOpponentPokemon->data->hp) + " points!";
                battleDialogues.push_back(newXPSentence);
                turnActionQueue.push_back("PLAYER_GAIN_SCORE");

                opponentFaintedPokemons++;
                if (opponentFaintedPokemons < 3) {
                    std::string newSentence = battleOpponent->name + " sent out " + battleOpponent->party[opponentFaintedPokemons].data->name + "!";
                    battleDialogues.push_back(newSentence);
                    turnActionQueue.push_back("OPPONENT_NEXT_POKEMON");
                } else {
                    std::string newSentence = battlePlayer->getPlayerName() + " defeated " + battleOpponent->name + "!";
                    battleDialogues.push_back(newSentence);
                    turnActionQueue.push_back("OPPONENT_DEFEATED");
                    battleDialogues.push_back(" ");
                    turnActionQueue.push_back("END_BATTLE");
                }
            }
        }
    }
}

void BattleScreen::localSwitchPokemonHandler(int selPoke) {
    if (battlePlayer->party[selPoke].c_hp == 0 or currentPlayerPokemon == &(battlePlayer->party[selPoke])) {
        Mix_PlayChannel(-1, deniedSound, 0);
    } else {
        Mix_PlayChannel(-1, aButton, 0);
        inSelectionScreen = false;
        if (currentPlayerPokemon->c_hp == 0) { // IF THE PLAYER SENDS OUT THE NEXT POKEMON AFTER A FAINT
            currentPlayerPokemon = &(battlePlayer->party[selPoke]);
            std::string newSentence = battlePlayer->getPlayerName() + " sent out " + battlePlayer->party[selPoke].data->name + "!";
            battleDialogues.push_back(newSentence);
            turnActionQueue.push_back("PLAYER_NEXT_POKEMON");
            SDL_DestroyTexture(playerPokeTexture);
            std::string imgPlayerPath = "res/pokemonassets/" + currentPlayerPokemon->data->name + ".png";
            playerPokeTexture = IMG_LoadTexture(RenderWindow::renderer, imgPlayerPath.c_str());
            BattleSen++;
        } else { // THE PLAYER WITHDRAWS THE POKEMON FOR A NEW ONE
            std::string newSentence = battlePlayer->getPlayerName() + " withdrew " + currentPlayerPokemon->data->name + "!";
            battleDialogues.push_back(newSentence);
            turnActionQueue.push_back("PLAYER_WITHDREW_POKEMON");
            showPHPBar = false;
            fightScreen = false;
            currentPlayerPokemon = &(battlePlayer->party[selPoke]);
            newSentence = battlePlayer->getPlayerName() + " sent out " + battlePlayer->party[selPoke].data->name + "!";
            battleDialogues.push_back(newSentence);
            turnActionQueue.push_back("PLAYER_NEXT_POKEMON");
            playerBeforeAttackHP = currentPlayerPokemon->c_hp;
            bool isKO = useMove(computerChooseMove(*currentOpponentPokemon, *currentPlayerPokemon), *currentOpponentPokemon, *currentPlayerPokemon, true);
            if (isKO) {
                std::string newSentence = currentPlayerPokemon->data->name + " fainted!";
                battleDialogues.push_back(newSentence);
                turnActionQueue.push_back("PLAYER_FAINT");
                playerFaintedPokemons++;
                if (playerFaintedPokemons < 3) {
                    battleDialogues.push_back("Awaiting " + battlePlayer->getPlayerName() + "'s next Pokemon...");
                    turnActionQueue.push_back("FORCE_OPEN_PARTY");
                } else {
                    battleDialogues.push_back(battlePlayer->getPlayerName() + " lost to " + battleOpponent->name + "!");
                    turnActionQueue.push_back("PLAYER_DEFEATED");
                    battleDialogues.push_back(" ");
                    turnActionQueue.push_back("END_BATTLE");
                }
            }
        }
    }
}

// BATTLE SCREEN BUTTONS

BattleScreenButton::BattleScreenButton() {
    buttonTexture = NULL;
    clickedOn = false;
    currentButtonFrame = 0;
}

BattleScreenButton::~BattleScreenButton() {
    freeButton();
}

void BattleScreenButton::freeButton() {
    if (buttonTexture != NULL) SDL_DestroyTexture(buttonTexture);
    buttonTexture = NULL;
    clickedOn = false;
    currentButtonFrame = 0;
    moveNames.freeText();
}

void BattleScreenButton::initBSB(const char* path, int x, int y, int BW, int BH, int imgWidth, int imgHeight, bool isMoveButton) {
    freeButton();
    buttonTexture = IMG_LoadTexture(RenderWindow::renderer, path);
    for (int i = 0; i < 3; i++) {
        buttonFrames[i] = {0, imgHeight*i, imgWidth, imgHeight};
    }
    buttonDest = {x, y, BW, BH};
    if (isMoveButton) {
        moveNames.createFont("res/font/gamefont.ttf", 28);
    }
}

void BattleScreenButton::drawButton(bool isMoveButton) {
    SDL_RenderCopy(RenderWindow::renderer, buttonTexture, &buttonFrames[currentButtonFrame], &buttonDest);
    if (isMoveButton == true) {
        moveNames.display(buttonDest.x + 12, buttonDest.y + 20, RenderWindow::renderer);
    }
}

void BattleScreenButton::buttonHandler() {
    // 0: INSIDE, 1: HOVER, 2: CLICK
    if (e.type == SDL_MOUSEMOTION or e.type == SDL_MOUSEBUTTONDOWN or e.type == SDL_MOUSEBUTTONUP) {
        int x, y;
        eventMousePosition(e, x, y);

        bool inside = true;
        if (x < buttonDest.x) inside = false;
        else if (x > buttonDest.x + buttonDest.w) inside = false;
        else if (y < buttonDest.y) inside = false;
        else if (y > buttonDest.y + buttonDest.h) inside = false;

        if (inside == false) {
            currentButtonFrame = 0;
        } else {
            switch (e.type) {
                case SDL_MOUSEMOTION:
                currentButtonFrame = 1;
                break;

                case SDL_MOUSEBUTTONDOWN:
                currentButtonFrame = 2;
                break;

                case SDL_MOUSEBUTTONUP:
                currentButtonFrame = 1;
                clickedOn = true;
                break;

                default:
                break;
            }
        }
    }
}

void BattleScreenButton::moveButtonHandler(int buttonNum) {
    int x, y;
    SDL_GetMouseState(&x, &y);

    bool inside = true;
    if (x < buttonDest.x) inside = false;
    else if (x > buttonDest.x + buttonDest.w) inside = false;
    else if (y < buttonDest.y) inside = false;
    else if (y > buttonDest.y + buttonDest.h) inside = false;

    if (inside == false) {
        moveNames.textInit(RenderWindow::renderer, mainBattle.getCurrentPlayerPokemon()->data->move[buttonNum]->name.c_str(), {0, 0, 0}, buttonDest.x + 278);
        currentButtonFrame = 0;
    } else {
        std::string moveDesc;
        moveDesc = "PP: " + to_string(mainBattle.getCurrentPlayerPokemon()->c_pp[buttonNum]) + "/" + to_string(mainBattle.getCurrentPlayerPokemon()->data->move[buttonNum]->pp);
        moveNames.textInit(RenderWindow::renderer, moveDesc.c_str(), {0, 0, 0}, buttonDest.x + 278);
    }
}

// POKEMON SELECTION SCREEN

PokemonSelectionScreen::PokemonSelectionScreen() {
    pokePartyScreenText = NULL;
    canBattleTexture = NULL;
    cannotBattleTexture = NULL;
    HPBarTexture = NULL;
    hpColor = NULL;
}

PokemonSelectionScreen::~PokemonSelectionScreen() {
    freeSelectionScreen();
}

void PokemonSelectionScreen::initSelectionScreen(mPlayer* player) {
    pokePartyScreenText = IMG_LoadTexture(RenderWindow::renderer, "res/battleassets/pokemon_sel_screen.png");
    pokePartyScreenRect = {91, 152, 650, 400};

    whichPoke.createFont("res/font/gamefont.ttf", 30);
    whichPoke.textInit(RenderWindow::renderer, "Which Pokemon to sub in?", {0, 0, 0}, 710);
    
    canBattleTexture = IMG_LoadTexture(RenderWindow::renderer, "res/battleassets/canBattleIcon.png");
    cannotBattleTexture = IMG_LoadTexture(RenderWindow::renderer, "res/battleassets/cannotBattleIcon.png");
    pokeBallRect[0] = {124, 244, 36, 36};
    pokeBallRect[1] = {124, 322, 36, 36};
    pokeBallRect[2] = {124, 400, 36, 36};

    HPBarTexture = IMG_LoadTexture(RenderWindow::renderer, "res/battleassets/inPartyHP.png");
    hpBarRect[0] = {500, 280, 168, 18};
    hpBarRect[1] = {500, 358, 168, 18};
    hpBarRect[2] = {500, 436, 168, 18};

    hpColor = IMG_LoadTexture(RenderWindow::renderer, "res/battleassets/HPColor.png");
    pokemonHP[0] = {541, 284, int(ceil(121.0*(double(player->party[0].c_hp)/double(player->party[0].data->hp)))), 8};
    pokemonHP[1] = {541, 362, int(ceil(121.0*(double(player->party[1].c_hp)/double(player->party[1].data->hp)))), 8};
    pokemonHP[2] = {541, 440, int(ceil(121.0*(double(player->party[2].c_hp)/double(player->party[2].data->hp)))), 8};

    for (int i = 0; i < 3; i++) {
        pokeNames[i].createFont("res/font/gamefont.ttf", 42);
        pokeNames[i].textInit(RenderWindow::renderer, (player->party[i].data->name).c_str(), {0, 0, 0}, 710);

        pokeTextHPs[i].createFont("res/font/gamefont.ttf", 28);
        std::string newHPText = to_string(player->party[i].c_hp) + "/" + to_string(player->party[i].data->hp); 
        pokeTextHPs[i].textInit(RenderWindow::renderer, newHPText.c_str(), {0,0,0}, 710);
    }

    pokemonSelectionButton[0].initBSB("res/battleassets/backbutton.png", 123, 242, 547, 58, 0, 0);
    pokemonSelectionButton[1].initBSB("res/battleassets/backbutton.png", 123, 320, 547, 58, 0, 0);
    pokemonSelectionButton[2].initBSB("res/battleassets/backbutton.png", 123, 398, 547, 58, 0, 0);
    backButton.initBSB("res/battleassets/backbutton.png", 592, 473, 113, 43, 362, 139);
}

void PokemonSelectionScreen::freeSelectionScreen() {
    SDL_Texture** textures[] = {&pokePartyScreenText, &canBattleTexture, &cannotBattleTexture, &HPBarTexture, &hpColor};
    for (SDL_Texture** texture : textures) {
        if (*texture != NULL) SDL_DestroyTexture(*texture);
        *texture = NULL;
    }
    whichPoke.freeText();
    for (int i = 0; i < 3; i++) {
        pokeNames[i].freeText();
        pokeTextHPs[i].freeText();
        pokemonSelectionButton[i].freeButton();
    }
    backButton.freeButton();
}

void PokemonSelectionScreen::updateSelectionScreen(mPlayer* player) {
    pokemonHP[0] = {541, 284, int(ceil(121.0*(double(player->party[0].c_hp)/double(player->party[0].data->hp)))), 8};
    pokemonHP[1] = {541, 362, int(ceil(121.0*(double(player->party[1].c_hp)/double(player->party[1].data->hp)))), 8};
    pokemonHP[2] = {541, 440, int(ceil(121.0*(double(player->party[2].c_hp)/double(player->party[2].data->hp)))), 8};

    for (int i = 0; i < 3; i++) {
        std::string newHPText = to_string(player->party[i].c_hp) + "/" + to_string(player->party[i].data->hp); 
        pokeTextHPs[i].textInit(RenderWindow::renderer, newHPText.c_str(), {0,0,0}, 710);
    }
}

void PokemonSelectionScreen::display() {
    SDL_RenderCopy(RenderWindow::renderer, pokePartyScreenText, NULL, &pokePartyScreenRect);

    whichPoke.display(123, 184, RenderWindow::renderer);

    pokeNames[0].display(173, 244, RenderWindow::renderer);
    pokeNames[1].display(173, 322, RenderWindow::renderer);
    pokeNames[2].display(173, 400, RenderWindow::renderer);

    for (int i = 0; i < 3; i++) {
        if (pokemonHP[i].w == 0) SDL_RenderCopy(RenderWindow::renderer, cannotBattleTexture, NULL, &pokeBallRect[i]);
        else SDL_RenderCopy(RenderWindow::renderer, canBattleTexture, NULL, &pokeBallRect[i]);

        SDL_RenderCopy(RenderWindow::renderer, HPBarTexture, NULL, &hpBarRect[i]);
        SDL_RenderCopy(RenderWindow::renderer, hpColor, NULL, &pokemonHP[i]);
    }
    pokeTextHPs[0].display(500, 244, RenderWindow::renderer);
    pokeTextHPs[1].display(500, 322, RenderWindow::renderer);
    pokeTextHPs[2].display(500, 400, RenderWindow::renderer);

    backButton.drawButton();
}