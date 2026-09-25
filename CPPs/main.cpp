#include "Variables.h"
#include "replay.h"
#include "rng.h"

// CORE GAME FUNCTIONS

bool initSystem();
void gameLoop();
void titleScreenInputProcess(SDL_Event* e);
void battleInputProcess(SDL_Event *e);
void overworldInputProcess(SDL_Event* e, int pCX, int pCY);
void freeMainAssets();

int main(int argc, char *argv[]) {
    initSystem();
    gameLoop();
    freeMainAssets();
    renderWindow.close();
    return 0;
}

bool initSystem() {
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    renderWindow.create("Pokémon UET");
    replay::load();

    // LOAD SAVE FILE
    if (!mainPlayer.loadPlayerData()) {
        hasSaveFile = false;
        inTitleScreen = true;
    } else {
        mainPlayer.initPlayerTexture();
        mainCamera.setCameraPos((mainPlayer.getXCoords() - 6) * 64, (mainPlayer.getYCoords() - 5) * 64);
    }

    // INIT THE TITLE SCREEN
    gameTitleScreen.initTitleScreen(hasSaveFile);
    gameTitleScreen.tsButtonInit();

    // INIT THE SETUP SCREEN
    mainSetup.initSetupScreen();

    // INIT THE PLAYER'S CURRENT MAP
    playerMap = new Map;
    playerMap->loadMap(gameMaps[mainPlayer.getCurrentMap()].c_str(), gameTileSets[mainPlayer.getCurrentMap()].c_str(), gameThemes[mainPlayer.getCurrentMap()].c_str(), themeRepeats[mainPlayer.getCurrentMap()], mapOverlays[mainPlayer.getCurrentMap()]);

    // INIT THE OVERLAY ELEMENTS TEXTURE
    playerMap->initOverlayElements("res/tileset/overlaying_tiles.png");

    // LOAD TRANSITION EFFECT
    blackTransitionTexture = renderWindow.loadTexture("res/otherassets/blacktransition.png");
    SDL_SetTextureBlendMode(blackTransitionTexture, SDL_BLENDMODE_BLEND);

    // INIT THE SOUND EFFECTS
    gameMusic.loadMusic(gameThemes[mainPlayer.getCurrentMap()].c_str(), themeRepeats[mainPlayer.getCurrentMap()]);
    changeMap = Mix_LoadWAV("res/sfx/change_map.wav");
    aButton = Mix_LoadWAV("res/sfx/a_button.wav");
    gameSaved = Mix_LoadWAV("res/sfx/game_saved.wav");
    startMenuSound = Mix_LoadWAV("res/sfx/start_menu.wav");
    deniedSound = Mix_LoadWAV("res/sfx/denied.wav");
    clickedOnSound = Mix_LoadWAV("res/sfx/clicked_on_sound.wav");

    // INIT THE MAIN MENU
    mainMenu.initMenu();

    // INIT THE DIALOGUE BOX TEXTURE
    d_box.initDialogueBox(RenderWindow::renderer, "res/otherassets/dialoguebox.png");
    d_text.createFont("res/font/gamefont.ttf", 38);
    return true;
}

// Releases every SDL resource held by the global game objects. Must run before
// renderWindow.close() shuts SDL down; the globals' destructors run after that and
// only see already-released (NULL) handles.
void freeMainAssets() {
    gameTitleScreen.freeTitleScreen();
    mainMenu.freeMenu();
    mainSetup.freeSetupScreen();
    mainBattle.freeBattleScreen();
    mainPlayer.freePlayer();
    if (playerMap != NULL) {
        playerMap->freeOverlayElements();
        delete playerMap;
        playerMap = NULL;
    }

    if (blackTransitionTexture != NULL) SDL_DestroyTexture(blackTransitionTexture);
    blackTransitionTexture = NULL;

    Mix_HaltMusic();
    gameMusic.freeMusic();

    Mix_Chunk** chunks[] = {&changeMap, &aButton, &gameSaved, &startMenuSound, &deniedSound, &clickedOnSound};
    for (Mix_Chunk** chunk : chunks) {
        if (*chunk != NULL) Mix_FreeChunk(*chunk);
        *chunk = NULL;
    }

    d_box.freeDialogueBox();
    d_text.freeText();
}

void overworldInputProcess(SDL_Event* e, int pCX, int pCY) {
    while (SDL_PollEvent(e)) {
        if (e->type == SDL_QUIT) // QUIT GAME
        {
            mainPlayer.savePlayerData();
            quit = true;
        }

        else if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_v && e->key.repeat == 0 && beginMapToBattleTransition == false && beginMapToMapTransition == false) // OPEN MENU
        { 
            inMenu = true;
            Mix_PlayChannel(-1, startMenuSound, 0);
        }
 
        else if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_x && e->key.repeat == 0 && inMenu == false && beginMapToBattleTransition == false && beginMapToMapTransition == false) // INTERACT WITH NPCS AND BLOCKS
        {
            NPC* selNPC = playerMap->getNearbyNPC(pCX, pCY, mainPlayer.getFacingDirection());
            if (selNPC != NULL) {
                if (selNPC->talkNPC(mainPlayer.getFacingDirection()) == true) {
                    inDialogue = true;
                }
                else inDialogue = false;
                Mix_PlayChannel(-1, aButton, 0);
            } else {
                InterTile* selInterTile = playerMap->getNearbyInterTile(pCX, pCY, mainPlayer.getFacingDirection());
                if (selInterTile != NULL) {
                    if (selInterTile->talkTile() == true) {
                        inDialogue = true;
                    }
                    else inDialogue = false;
                    Mix_PlayChannel(-1, aButton, 0);
                }
            }
        } 

        else if (e->type == SDL_KEYDOWN and mainCamera.getMovementState() == false and e->key.repeat == 0 and inDialogue == false and inMenu == false and beginMapToMapTransition == false and finishMapToMapTransition == false) // BEGIN MOVEMENT
        {
            switch (e->key.keysym.sym) {
                case SDLK_s:
                    mainPlayer.changeFacingDirect(0);
                    break;
                case SDLK_d:
                    mainPlayer.changeFacingDirect(1);
                    break;
                case SDLK_w:
                    mainPlayer.changeFacingDirect(2);
                    break;
                case SDLK_a:
                    mainPlayer.changeFacingDirect(3);
                    break;
                default:
                    break;
            }
            WarpTile* selWarpTile = playerMap->getNearbyWarpTile(pCX, pCY, mainPlayer.getFacingDirection());
            if (selWarpTile != NULL and (e->key.keysym.sym == SDLK_w or e->key.keysym.sym == SDLK_a or e->key.keysym.sym == SDLK_s or e->key.keysym.sym == SDLK_d)) {
                beginMapToMapTransition = true;
            } else {
                mainCamera.beginMovement(e, pCX, pCY, playerMap->getCollisionMap(), playerMap->getMapWidth(), playerMap->getMapHeight());
            }
        } 

        else if (e->type == SDL_KEYUP and mainCamera.getMovementState() == true and e->key.repeat == 0) // STOP MOVEMENT 
        { 
            mainCamera.stopMovement(e);
        }

        if (inMenu) {
            mainMenu.centralMenuInputProcess();
        }
    }
}

void battleInputProcess(SDL_Event* e) // THE BATTLE INPUT PROCESS
{
    while (SDL_PollEvent(e)) {
        if (e->type == SDL_QUIT) {
            quit = true;
        }
        mainBattle.centralBattleProcess(e);
    }
}

void titleScreenInputProcess(SDL_Event* e) // ALREADY MOSTLY FINISHED. DO NOT TOUCH.
{ 
    while (SDL_PollEvent(e)) {
        if (e->type == SDL_QUIT) {
            quit = true;
            gameTitleScreen.freeTitleScreen();
        }
        if (gameTitleScreen.acceptInputState() == true)
        {
            gameTitleScreen.doButtonEvents(e);
            if (hasSaveFile == true and gameTitleScreen.tsButtons[0].isClicked() == true) {
                Mix_PlayChannel(-1, clickedOnSound, 0);
                gameTitleScreen.tsButtons[0].resetClickState();
                gameTitleScreen.stopInputState();
                tsToMapTransition = true;
            } else if (gameTitleScreen.tsButtons[1].isClicked() == true) {
                Mix_PlayChannel(-1, clickedOnSound, 0);
                gameTitleScreen.tsButtons[1].resetClickState();
                
                playerMap->freeMap();
                playerMap->loadMap(gameMaps[1].c_str(), gameTileSets[1].c_str(), gameThemes[1].c_str(), themeRepeats[1], mapOverlays[1]);
                mainPlayer.resetPlayerData();

                mainCamera.setCameraPos((mainPlayer.getXCoords() - 6) * 64, (mainPlayer.getYCoords() - 5) * 64);
                gameTitleScreen.stopInputState();
                gameMusic.loadMusic(gameThemes[mainPlayer.getCurrentMap()].c_str(), themeRepeats[mainPlayer.getCurrentMap()]);
                startTSToSetupTransition = true;
            } else if (gameTitleScreen.tsButtons[2].isClicked() == true) {
                Mix_PlayChannel(-1, clickedOnSound, 0);
                gameTitleScreen.tsButtons[2].resetClickState();
                gameTitleScreen.toggleHelpScreen();
            } else if (gameTitleScreen.tsButtons[3].isClicked() == true) {
                Mix_PlayChannel(-1, clickedOnSound, 0);
                quit = true;
                gameTitleScreen.freeTitleScreen();
            } 
        }
    }
}

void gameLoop() {
    bool transitionMusicLoaded = false;
    while (quit == false) {
        if (inTitleScreen == true) // PLAYER IN THE TITLE SCREEN. AGAIN, MOSTLY FINISHED, DO NOT TOUCH
        {
            titleScreenInputProcess(&e);
            
            renderWindow.drawColor(0, 0, 0);
            renderWindow.clear();            

            gameTitleScreen.drawTitleScreen();

            if (tsToMapTransition == true) {
                if (transitionTransparency < 255) {
                    transitionTransparency += 17;
                } else if (transitionTransparency >= 255) {
                    inTitleScreen = false;
                    gameTitleScreen.freeTitleScreen();
                    replay::delay(1000);
                }
                SDL_SetTextureAlphaMod(blackTransitionTexture, transitionTransparency);
                SDL_RenderCopy(RenderWindow::renderer, blackTransitionTexture, NULL, NULL);
            }

            if (startTSToSetupTransition == true) {
                if (transitionTransparency < 255) {
                    transitionTransparency += 17;
                } else if (transitionTransparency >= 255) {
                    inTitleScreen = false;
                    inSetupScreen = true;
                    startTSToSetupTransition = false;
                    finishTSToSetupTransition = true;
                    gameTitleScreen.freeTitleScreen();
                    replay::delay(500);
                }
                SDL_SetTextureAlphaMod(blackTransitionTexture, transitionTransparency);
                SDL_RenderCopy(RenderWindow::renderer, blackTransitionTexture, NULL, NULL);
            }

            renderWindow.display();

            replay::delay(1000/60);
        }

        else if (inSetupScreen == true)
        {
            while (SDL_PollEvent(&e)) mainSetup.setupScreenInputProcess(&e);

            renderWindow.drawColor(0, 0, 0);
            renderWindow.clear();

            mainSetup.drawSetupScreen();

            if (finishTSToSetupTransition == true) {
                if (transitionTransparency > 0) {
                    transitionTransparency -= 17;
                } else if (transitionTransparency <= 0) {
                    transitionTransparency = 0;
                    finishTSToSetupTransition = false;
                }
                SDL_SetTextureAlphaMod(blackTransitionTexture, transitionTransparency);
                SDL_RenderCopy(RenderWindow::renderer, blackTransitionTexture, NULL, NULL);
            }

            if (tsToMapTransition == true) {
                if (transitionTransparency < 255) {
                    transitionTransparency += 17;
                } else if (transitionTransparency >= 255) {
                    inSetupScreen = false;
                    replay::delay(1000);
                }
                SDL_SetTextureAlphaMod(blackTransitionTexture, transitionTransparency);
                SDL_RenderCopy(RenderWindow::renderer, blackTransitionTexture, NULL, NULL);
            }

            renderWindow.display();

            replay::delay(1000 / 60);
        }
        
        else if (inBattle == true) // PLAYER IN BATTLE
        {
            battleInputProcess(&e);

            renderWindow.drawColor(255, 0, 255);
            renderWindow.clear();

            mainBattle.drawBattleScreen(finishMapToBattleTransition, beginBattleToMapTransition);

            if (finishMapToBattleTransition == true) {
                if (transitionTransparency > 0) {
                    transitionTransparency -= 5;
                } else if (transitionTransparency <= 0) {
                    transitionTransparency = 0;
                    finishMapToBattleTransition = false;
                }
                SDL_SetTextureAlphaMod(blackTransitionTexture, transitionTransparency);
                SDL_RenderCopy(RenderWindow::renderer, blackTransitionTexture, NULL, NULL);
            }

            if (beginBattleToMapTransition == true) {
                if (transitionTransparency < 255) {
                    transitionTransparency += 5;
                } else if (transitionTransparency >= 255) {
                    transitionTransparency = 255;
                    inBattle = false;
                    beginBattleToMapTransition = false;
                    finishBattleToMapTransition = true;
                }
                SDL_SetTextureAlphaMod(blackTransitionTexture, transitionTransparency);
                SDL_RenderCopy(RenderWindow::renderer, blackTransitionTexture, NULL, NULL);
            }

            gameMusic.play();

            renderWindow.display();

            replay::delay(1000 / 60);
        }

        else // PLAYER IN THE MAP

        {
            int pCX = mainPlayer.getXCoords(), pCY = mainPlayer.getYCoords(); // GET PLAYER COORDS
            overworldInputProcess(&e, pCX, pCY); // INPUT PROCESS

            const Uint8 *currentKeyStates = SDL_GetKeyboardState(NULL);
            if (currentKeyStates[SDL_SCANCODE_Z]) {mainCamera.speedUp(); playerIsRunning = true;} // TOGGLE RUNNING STATE
            else {mainCamera.slowDown(); playerIsRunning = false;}

            // MOVE THE CAMERA
            if (mainCamera.getFinishingState() == true)
            { 
                mainCamera.finishMovement();
            } 
            else 
            {
                mainCamera.moveCamera(pCX, pCY, playerMap->getCollisionMap(), playerMap->getMapWidth(), playerMap->getMapHeight());
                mainCamera.finishIllegalPos(playerMap->getMapWidth(), playerMap->getMapHeight());
            }

            // UPDATE THE PLAYER COORDINATES
            mainPlayer.setPlayerCoords(mainCamera.getCamX() / 64 + 6, mainCamera.getCamY() / 64 + 5, playerMap->getMapID()); 

            // PRPARE THE RENDERING SCREEN
            renderWindow.drawColor(0, 0, 0); 
            renderWindow.clear();

            playerMap->drawMap(&mainCamera); // DRAW THE MAP
            playerMap->drawNPCs(&mainCamera); // DRAW THE MAP'S NPC

            // DRAW THE PLAYER
            if (mainCamera.getMovementState() == false) mainPlayer.renderStandingPlayer();  
            else {
                if (playerIsRunning == true) mainPlayer.renderRunningPlayer(); 
                else mainPlayer.renderMovingPlayer();
            }

            playerMap->drawFrontNPCs(&mainCamera); // DRAW NPCs THAT ARE IN FRONT OF THE PLAYER

            // DRAW THE OVERLAYING ELEMENTS IF THE MAP HAS THEM
            if (mapOverlays[mainPlayer.getCurrentMap()] == true) {
                playerMap->drawOverlay(&mainCamera);
            }
            
            // NPCDIALOGUE and INTERTILEDIALOGUE
            if (inDialogue == true) {
                d_box.renderDialogueBox(RenderWindow::renderer, &dBoxClip);
                NPC* selNPC = playerMap->getNearbyNPC(pCX, pCY, mainPlayer.getFacingDirection());
                if (selNPC != NULL) {
                    d_text.textInit(RenderWindow::renderer, selNPC->getCurrentSentence().c_str(), { 0, 0, 0 });
                    d_text.display(48, 547, RenderWindow::renderer);
                }
                else {
                    InterTile* selInterTile = playerMap->getNearbyInterTile(pCX, pCY, mainPlayer.getFacingDirection());
                    if (selInterTile != NULL) {
                        d_text.textInit(RenderWindow::renderer, selInterTile->getInterCurrentSentence().c_str(), { 0, 0, 0 });
                        d_text.display(48, 547, RenderWindow::renderer);
                        if (selInterTile->getX() == 13 && selInterTile->getY() == 16 && mainPlayer.getCurrentMap() == 4) { // SPECIAL TILE FOR GENERATING NEW POKEMONS UPON TALKING TO
                            if (selInterTile->getInterCurrentSentenceID() == 2) {
                                mainPlayer.party[0] = randomInt(1, psize - 1);
                                mainPlayer.party[1] = randomInt(1, psize - 1);
                                mainPlayer.party[2] = randomInt(1, psize - 1);
                            } else if (selInterTile->getInterCurrentSentenceID() == 1 and selInterTile->getInterDialogueSize() > 3) {
                                selInterTile->dialogueTexts.pop_back();
                            } else if (selInterTile->getInterCurrentSentenceID() == 3 and selInterTile->getInterDialogueSize() < 4) {
                                std::string newPokemonSentence = mainPlayer.getPlayerName() + " received " + mainPlayer.party[0].data->name + ", " + mainPlayer.party[1].data->name + " and " + mainPlayer.party[2].data->name + "!";
                                selInterTile->dialogueTexts.push_back(newPokemonSentence);
                            }
                        }
                    }
                }
            }

            if (inMenu == true) {
                mainMenu.drawMenu();
            }

            // ONLY ACTIVATED GOING FROM THE TITLE SCREEN TO THE OVERWORLD. SMOOTH BLACK TRANSITION
            if (tsToMapTransition == true) { 
                if (transitionTransparency > 0) {
                    transitionTransparency -= 17;
                } else if (transitionTransparency <= 0) {
                    tsToMapTransition = false;
                }
                SDL_SetTextureAlphaMod(blackTransitionTexture, transitionTransparency);
                SDL_RenderCopy(RenderWindow::renderer, blackTransitionTexture, NULL, NULL);
            }

            // MAP TO MAP TRANSITION HANDLING
            if (beginMapToMapTransition == true) {
                if (transitionTransparency < 255) {
                    transitionTransparency += 17;
                    if (transitionTransparency == 51) Mix_PlayChannel(-1, changeMap, 0);
                } else if (transitionTransparency >= 255) {
                    WarpTile* selWarpTile = playerMap->getNearbyWarpTile(pCX, pCY, mainPlayer.getFacingDirection());
                    if (selWarpTile == NULL) { // should not happen: the transition only starts while facing a warp tile
                        beginMapToMapTransition = false;
                        finishMapToMapTransition = true;
                    } else {
                    int tdM = selWarpTile->getDestMap(), tdX = selWarpTile->getDestX(), tdY = selWarpTile->getDestY();
                    beginMapToMapTransition = false;
                    finishMapToMapTransition = true;
                    if (gameThemes[tdM] != gameThemes[mainPlayer.getCurrentMap()]) {
                        Mix_FadeOutMusic(500);
                        gameMusic.freeMusic();
                        gameMusic.loadMusic(gameThemes[tdM].c_str(), themeRepeats[tdM]);
                        gameMusic.resetChord();
                    }
                    playerMap->freeMap();
                    playerMap->loadMap(gameMaps[tdM].c_str(), gameTileSets[tdM].c_str(), gameThemes[tdM].c_str(), themeRepeats[tdM], mapOverlays[tdM]);
                    mainPlayer.setPlayerCoords(tdX, tdY, tdM);
                    mainCamera.setCameraPos((mainPlayer.getXCoords() - 6) * 64, (mainPlayer.getYCoords() - 5) * 64);
                    }
                }
                SDL_SetTextureAlphaMod(blackTransitionTexture, transitionTransparency);
                SDL_RenderCopy(RenderWindow::renderer, blackTransitionTexture, NULL, NULL);
            }

            // FINISH THE MAP TRANSITION EFFECT
            if (finishMapToMapTransition == true) {
                if (transitionTransparency > 0) {
                    transitionTransparency -= 17;
                } else if (transitionTransparency <= 0) {
                    finishMapToMapTransition = false;
                    mainPlayer.savePlayerData();
                }
                SDL_SetTextureAlphaMod(blackTransitionTexture, transitionTransparency);
                SDL_RenderCopy(RenderWindow::renderer, blackTransitionTexture, NULL, NULL);
            }

            // MAP TO BATTLE TRANSITION HANDLING
            if (beginMapToBattleTransition == true) {
                if (transitionMusicLoaded == false) { // load once, at the start of the transition
                    gameMusic.freeMusic();
                    gameMusic.resetChord();
                    gameMusic.loadMusic("res/music/battleMusic.mp3", 19.9);
                    transitionMusicLoaded = true;
                }
                if (transitionTransparency < 255) {
                    transitionTransparency += 5;
                } else if (transitionTransparency >= 255) {
                    transitionMusicLoaded = false;
                    NPC* selNPC = playerMap->getNearbyNPC(pCX, pCY, mainPlayer.getFacingDirection());
                    if (selNPC == NULL) { // should not happen: the battle only starts while facing its trainer
                        beginMapToBattleTransition = false;
                        tsToMapTransition = true; // fade back in
                    } else {
                    // Look up the trainer before removing the challenge room's gate NPC (always the last one).
                    if (mainPlayer.getCurrentMap() >= 5 and mainPlayer.getCurrentMap() <= 11) playerMap->popLastNPC();

                    defaultOppo.name = selNPC->getTrainerName();
                    defaultOppo.battleSpritePath = selNPC->getTrainerSprite();

                    defaultOppo.party[0] = randomInt(1, psize - 1);
                    defaultOppo.party[1] = randomInt(1, psize - 1);
                    defaultOppo.party[2] = randomInt(1, psize - 1);

                    transitionTransparency = 255;
                    inBattle = true;
                    beginMapToBattleTransition = false;
                    finishMapToBattleTransition = true;
                    mainBattle.initBattleScreen(&mainPlayer, &defaultOppo);
                    
                    selNPC->changeBattleStatus(1);
                    }
                }
                SDL_SetTextureAlphaMod(blackTransitionTexture, transitionTransparency);
                SDL_RenderCopy(RenderWindow::renderer, blackTransitionTexture, NULL, NULL);
            }

            // FINISHING BATTLE TRANSITION HANDLING
            if (finishBattleToMapTransition == true) {
                if (transitionMusicLoaded == false) { // load once, at the start of the transition
                    gameMusic.freeMusic();
                    gameMusic.resetChord();
                    gameMusic.loadMusic(gameThemes[mainPlayer.getCurrentMap()].c_str(), themeRepeats[mainPlayer.getCurrentMap()]);
                    transitionMusicLoaded = true;
                }

                if (transitionTransparency > 0) {
                    transitionTransparency -= 5;
                } else if (transitionTransparency <= 0) {
                    transitionMusicLoaded = false;
                    transitionTransparency = 0;
                    finishBattleToMapTransition = false;
                    mainBattle.freeBattleScreen();
                    for (int i = 0; i < 3; i++) mainPlayer.party[i] = mainPlayer.party[i].data - pokemonData;
                }
                SDL_SetTextureAlphaMod(blackTransitionTexture, transitionTransparency);
                SDL_RenderCopy(RenderWindow::renderer, blackTransitionTexture, NULL, NULL);
            }

            if (beginMapToMapTransition == false and finishBattleToMapTransition == false) gameMusic.play(); // PLAY THE MAP'S THEME
            
            renderWindow.display(); // DISPLAY THE CONTENT TO THE WINDOW

            replay::delay(1000 / 60); // PRIMITIVE CAP FRAME RATE
        }
    }
}