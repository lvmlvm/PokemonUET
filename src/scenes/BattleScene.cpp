#include "scenes/BattleScene.h"
#include "ui/Colors.h"

#include <cmath>

namespace {
const int FADE_STEP = 5;
const double PI = 3.141592;
} // namespace

BattleScene::BattleScene(Game& game, Trainer& opponent, std::function<void()> respawn)
    : game_(game),
      renderer_(game.renderer()),
      player_(game.player()),
      opponent_(opponent),
      respawn_(std::move(respawn)),
      currentPlayerPokemon(&player_.party[0]),
      currentOpponentPokemon(&opponent.party[0]),
      partyView_(game.assets(), PartyView::Style::Battle) {
    Assets& assets = game.assets();

    background_ = assets.texture("battleassets/battlebackground0.png");
    circle_ = assets.texture("battleassets/battlecircle0.png");
    grayBox_ = assets.texture("battleassets/graybox.png");
    playerHPBar_ = assets.texture("battleassets/pHPBar.png");
    oppoHPBar_ = assets.texture("battleassets/oHPBar.png");
    hpColor_ = assets.texture("battleassets/HPColor.png");
    playerTrainerTexture_ = player_.gender() == 0 ? assets.texture("battleassets/malefrontsprite.png")
                                                  : assets.texture("battleassets/femalefrontsprite.png");
    opponentTrainerTexture_ = assets.texture(opponent.battleSpritePath);
    playerPokeTexture_ = pokemonTexture(*currentPlayerPokemon);
    opponentPokeTexture_ = pokemonTexture(*currentOpponentPokemon);

    currPlayHP = {708, 443, playerHpWidth(currentPlayerPokemon->c_hp), 7};
    currOppoHP = {112, 119, opponentHpWidth(), 7};
    for (int i = 0; i < 4; i++) playerTextureFrames[i] = {i * 80, 0, 80, 80};

    // The intro: three lines, shown before the first turn.
    log_.dialogues = {
        "You are challenged by " + opponent.name + "!",
        opponent.name + " sent out " + currentOpponentPokemon->data->name + "!",
        player_.name() + " sent out " + currentPlayerPokemon->data->name + "!",
    };
    log_.actions = {"BATTLE_START"};

    dialogueText_ = TextLabel(renderer_, assets.font(42));
    playerPokeName_ = TextLabel(renderer_, assets.font(28));
    oppoPokeName_ = TextLabel(renderer_, assets.font(28));

    fightButton_ = Button::vertical(assets.texture("battleassets/fightbutton.png"), {230, 527, 371, 150}, 556, 224);
    pokemonButton_ = Button::vertical(assets.texture("battleassets/pokemonbutton.png"), {23, 559, 188, 86}, 282, 130);
    retireButton_ = Button::vertical(assets.texture("battleassets/retirebutton.png"), {621, 559, 188, 86}, 282, 130);
    const SDL_Point movePositions[4] = {{33, 519}, {344, 519}, {33, 611}, {344, 611}};
    for (int i = 0; i < 4; i++) {
        moveButtons_[i] = Button::vertical(assets.texture("battleassets/moveButton.png"), {movePositions[i].x, movePositions[i].y, 278, 72}, 652, 167);
        moveNames_[i] = TextLabel(renderer_, assets.font(28));
    }
    backButton_ = Button::vertical(assets.texture("battleassets/backbutton.png"), {659, 620, 154, 60}, 362, 139);

    effectiveSFX = assets.sound("sfx/normal_effective.wav");
    notEffectiveSFX = assets.sound("sfx/not_effective.wav");
    superEffectiveSFX = assets.sound("sfx/super_effective.wav");
    pokemonFainted = assets.sound("sfx/poke_fainted.wav");

    partyView_.update(player_.party);
}

TexturePtr BattleScene::pokemonTexture(const Pokemon& pokemon) {
    return game_.assets().loadTexture("pokemonassets/" + pokemon.data->name + ".png");
}

const std::string& BattleScene::actionAt(unsigned int i) const {
    static const std::string none;
    return i < log_.actions.size() ? log_.actions[i] : none;
}

const std::string& BattleScene::dialogueAt(unsigned int i) const {
    static const std::string none;
    return i < log_.dialogues.size() ? log_.dialogues[i] : none;
}

int BattleScene::playerHpWidth(int hp) const {
    return int(std::ceil(106.0 * (double(hp) / double(currentPlayerPokemon->data->hp))));
}

int BattleScene::opponentHpWidth() const {
    return int(std::ceil(107.0 * (double(currentOpponentPokemon->c_hp) / double(currentOpponentPokemon->data->hp))));
}

// FRAME

void BattleScene::frame() {
    SDL_SetRenderDrawColor(renderer_, 255, 0, 255, 255);
    SDL_RenderClear(renderer_);

    draw();

    Fade& fade = game_.fade();
    if (transition_ == Transition::FadeIn) {
        if (fade.in(FADE_STEP)) transition_ = Transition::None;
        fade.draw(renderer_);
    }
    if (transition_ == Transition::FadeOut) {
        if (fade.out(FADE_STEP)) {
            fade.setAlpha(255);
            transition_ = Transition::Done;
            game_.popScene();
        }
        fade.draw(renderer_);
    }

    game_.music().play();
}

void BattleScene::draw() {
    SDL_RenderCopy(renderer_, background_, nullptr, nullptr);

    if (transition_ != Transition::FadeIn) drawIntro();

    // THE TWO SIDES' POKEMON
    SDL_RenderCopy(renderer_, opponentPokeTexture_.get(), &halfOppoPokeRect, &oppoPokeRect);
    SDL_RenderCopy(renderer_, playerPokeTexture_.get(), &halfPlayerPokeRect, &playerPokeRect);

    // THE GRAY BOX HOLDING THE DIALOGUE
    if (grayBoxRect.y > 500) grayBoxRect.y -= 4;
    SDL_RenderCopy(renderer_, grayBox_, nullptr, &grayBoxRect);

    // THE INTRO LINES: the challenge, the opponent's Pokemon, the player's Pokemon
    if (transition_ != Transition::FadeIn && line_ == 0 && startingBattle) {
        dialogueText_.setText(dialogueAt(line_), TEXT_WHITE);
        dialogueText_.draw(32, 530);
    }

    if (line_ == 1 && startingBattle) {
        dialogueText_.setText(dialogueAt(line_), TEXT_WHITE);
        dialogueText_.draw(32, 530);
        if (opponentSpriteBox.x < 860) opponentSpriteBox.x += 8;
        if (opponentSpriteBox.x > 850 && oppoPokeRect.x > 500) {
            oppoPokeRect.x -= 8;
            if (oppoPokeRect.x < 510) {
                inAnim0 = false;
                showOHPBar = true;
            }
        }
    }

    if (line_ == 2 && startingBattle) {
        dialogueText_.setText(dialogueAt(line_), TEXT_WHITE);
        dialogueText_.draw(32, 530);
        if (playerSpriteBox.x > -240) {
            playerSpriteBox.x -= 8;
            if (playerSpriteBox.x == 70 || playerSpriteBox.x == 22 || playerSpriteBox.x == -26) currentPlayerFrame++; // throwing animation
        }
        if (playerSpriteBox.x < -230 && playerPokeRect.x < 90) {
            playerPokeRect.x += 8;
            if (playerPokeRect.x > 80) {
                inAnim0 = false;
                showPHPBar = true;
            }
        }
    }

    if (showOHPBar) {
        SDL_RenderCopy(renderer_, oppoHPBar_, nullptr, &oppoHPRect);
        SDL_RenderCopy(renderer_, hpColor_, nullptr, &currOppoHP);
        oppoPokeName_.setText(currentOpponentPokemon->data->name, TEXT_BLACK);
        oppoPokeName_.draw(4, 83);
    }

    if (showPHPBar) {
        SDL_RenderCopy(renderer_, playerHPBar_, nullptr, &playerHPRect);
        SDL_RenderCopy(renderer_, hpColor_, nullptr, &currPlayHP);
        playerPokeName_.setText(currentPlayerPokemon->data->name, TEXT_BLACK);
        playerPokeName_.draw(585, 407);
    }

    if (fightScreen) {
        fightButton_.draw(renderer_);
        pokemonButton_.draw(renderer_);
        retireButton_.draw(renderer_);
    }

    if (moveScreen) {
        for (int i = 0; i < 4; i++) {
            moveButtons_[i].draw(renderer_);
            moveNames_[i].draw(moveButtons_[i].rect().x + 12, moveButtons_[i].rect().y + 20);
        }
        backButton_.draw(renderer_);
    }

    if (!startingBattle && !fightScreen && !moveScreen) {
        dialogueText_.setText(dialogueAt(line_), TEXT_WHITE);
        dialogueText_.draw(32, 530);
        animateCurrentAction();
    }

    if (inSelectionScreen) partyView_.draw(renderer_);
}

// The trainers slide in on their battle circles.
void BattleScene::drawIntro() {
    // The circles follow parabolas over oCirX / pCirX = 0, 1, 2, ... until the opponent's
    // circle reaches x = 462.
    auto opponentCircleX = [this] { return -0.1276 * oCirX * oCirX + 20.58 * oCirX - 320; };
    if (opponentCircleX() < 462) {
        opponentSpriteBox.x = int(-0.1276 * oCirX * oCirX + 20.58 * oCirX - 300);
        opponentCircle.x = int(opponentCircleX());
        oCirX++;
    }
    SDL_RenderCopy(renderer_, circle_, nullptr, &opponentCircle);
    SDL_RenderCopy(renderer_, opponentTrainerTexture_, nullptr, &opponentSpriteBox);
    if (opponentCircleX() < 462) {
        playerCircle.x = int(0.1244 * pCirX * pCirX - 20.26 * pCirX + 832);
        playerSpriteBox.x = int(0.1244 * pCirX * pCirX - 20.26 * pCirX + 862);
        pCirX++;
        if (opponentCircleX() > 430) inAnim0 = false;
    }
    SDL_RenderCopy(renderer_, circle_, nullptr, &playerCircle);
    SDL_RenderCopy(renderer_, playerTrainerTexture_, &playerTextureFrames[currentPlayerFrame], &playerSpriteBox);
}

// Animates the log line being shown. inAnim0 is true while the animation runs; X only
// advances to the next line once it is done.
void BattleScene::animateCurrentAction() {
    const std::string& action = actionAt(line_);

    if (action == "PLAYER_USE_MOVE" || action == "OPPONENT_USE_MOVE") {
        const bool byPlayer = action == "PLAYER_USE_MOVE";
        double& moveAnim = byPlayer ? playerPokemonMoveAnim : opponentPokemonMoveAnim;
        SDL_Texture* target = byPlayer ? opponentPokeTexture_.get() : playerPokeTexture_.get();

        if (moveAnim < 21 && inAnim0) { // the attacker lunges forward and back
            if (byPlayer) playerPokeRect.x = -(moveAnim * moveAnim) + 20 * moveAnim + 90;
            else oppoPokeRect.x = (moveAnim * moveAnim) - 20 * moveAnim + 500;
            moveAnim++;
        } else if (moveAnim == 21 && inAnim0) {
            if (PokemonTransparency == 0) { // hit sound, depending on the effectiveness line that follows
                const std::string& next = actionAt(line_ + 1);
                if (next == "MOVE_NOEFFECT") {
                    moveAnim = 0;
                    PokemonTransparency = 0;
                    inAnim0 = false;
                } else if (next == "MOVE_NOT_EFFECTIVE") {
                    playSound(notEffectiveSFX);
                } else if (next == "MOVE_SUPER_EFFECTIVE") {
                    playSound(superEffectiveSFX);
                } else {
                    playSound(effectiveSFX);
                }
            }

            if (PokemonTransparency < 1170 && inAnim0) { // the target blinks
                SDL_SetTextureAlphaMod(target, int(255 * (std::abs(std::sin(PokemonTransparency * PI / 180.0)))));
                PokemonTransparency += 130;
            } else if (PokemonTransparency >= 1170 && inAnim0) {
                SDL_SetTextureAlphaMod(target, int(255 * (std::abs(std::sin(PokemonTransparency * PI / 180.0)))));
                moveAnim = 0;
                PokemonTransparency = 0;
                if (byPlayer) currOppoHP.w = opponentHpWidth();
                else currPlayHP.w = playerHpWidth(currentPlayerPokemon->c_hp);
                inAnim0 = false;
            }
        }
    }

    else if (action == "OPPONENT_FAINT") {
        if ((halfOppoPokeRect.h > 0 || oppoPokeRect.y < 260) && inAnim0) {
            halfOppoPokeRect.h -= 10;
            oppoPokeRect.y += 24;
            oppoPokeRect.h -= 24;
        } else if (halfOppoPokeRect.h <= 0 && oppoPokeRect.y == 260 && inAnim0) {
            playSound(pokemonFainted);
            showOHPBar = false;
            oppoPokeRect = {832, 20, 240, 240};
            halfOppoPokeRect.h = 96;
            inAnim0 = false;
        }
    }

    else if (action == "OPPONENT_NEXT_POKEMON") {
        if (oppoPokeRect.x > 500) {
            if (oppoPokeRect.x == 832) opponentPokeTexture_ = pokemonTexture(opponent_.party[opponentFaintedPokemons]);
            oppoPokeRect.x -= 8;
            if (oppoPokeRect.x < 510) {
                currentOpponentPokemon = &opponent_.party[opponentFaintedPokemons];
                inAnim0 = false;
                showOHPBar = true;
                currOppoHP.w = opponentHpWidth();
            }
        }
    }

    else if (action == "PLAYER_FAINT") {
        if ((halfPlayerPokeRect.h > 0 || playerPokeRect.y < 540) && inAnim0) {
            halfPlayerPokeRect.h -= 10;
            playerPokeRect.y += 24;
            playerPokeRect.h -= 24;
        } else if (halfPlayerPokeRect.h <= 0 && playerPokeRect.y == 540 && inAnim0) {
            playSound(pokemonFainted);
            showPHPBar = false;
            playerPokeRect = {-240, 300, 240, 240};
            halfPlayerPokeRect.h = 96;
            inAnim0 = false;
        }
    }

    else if (action == "FORCE_OPEN_PARTY") {
        inSelectionScreen = true;
        partyView_.update(player_.party);
        inAnim0 = false;
    }

    else if (action == "PLAYER_NEXT_POKEMON") {
        if (playerPokeRect.x < 90) {
            playerPokeRect.x += 8;
            if (playerPokeRect.x > 80) {
                inAnim0 = false;
                // After a voluntary switch the bar shows the HP from before the opponent's free hit.
                int hp = actionAt(line_ - 1) == "PLAYER_WITHDREW_POKEMON" ? playerBeforeAttackHP : currentPlayerPokemon->c_hp;
                currPlayHP = {708, 443, playerHpWidth(hp), 7};
                showPHPBar = true;
            }
        }
    }

    else if (action == "PLAYER_WITHDREW_POKEMON") {
        if (playerPokeRect.x > -240) {
            playerPokeRect.x -= 8;
            if (playerPokeRect.x < -230) {
                inAnim0 = false;
                playerPokeTexture_ = pokemonTexture(*currentPlayerPokemon);
            }
        }
    }

    else if (action == "PLAYER_DEFEATED") {
        player_.endRun();
        respawn_();
        inAnim0 = false;
    }

    else if (action == "END_BATTLE") {
        if (transition_ == Transition::None) transition_ = Transition::FadeOut;
    }

    else {
        inAnim0 = false;
    }
}

// INPUT

void BattleScene::handleEvent(const SDL_Event& e) {
    if (e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
        eventMousePosition(e, mouseX_, mouseY_);
    }

    // X ADVANCES THE BATTLE DIALOGUE
    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_x && e.key.repeat == 0 && !inAnim0 && !fightScreen && !moveScreen && !inSelectionScreen) {
        if (startingBattle) {
            if (line_ < 3) {
                inAnim0 = true;
                line_++;
                playSound(game_.sounds().aButton);
                if (line_ == 3) {
                    fightScreen = true;
                    startingBattle = false;
                    line_ = 0;
                    log_.clear();
                }
            }
        } else {
            inAnim0 = true;
            line_++;
            playSound(game_.sounds().aButton);
            if (line_ == log_.dialogues.size()) {
                fightScreen = true;
                line_ = 0;
                log_.clear();
            }
        }
        return;
    }

    // THE POKEMON SELECTION SCREEN (can't be closed when a switch is forced)
    if (inSelectionScreen) {
        const bool forced = actionAt(line_) == "FORCE_OPEN_PARTY";
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE && !forced) {
            inSelectionScreen = false;
            return;
        }

        Button& back = partyView_.backButton();
        back.handleEvent(e);
        if (back.clicked() && !forced) {
            back.resetClick();
            inSelectionScreen = false;
            return;
        }

        for (int i = 0; i < 3; i++) {
            Button& slot = partyView_.slotButton(i);
            slot.handleEvent(e);
            if (slot.clicked()) {
                slot.resetClick();
                pokemonButton_.handleEvent(e);
                switchPokemon(i);
                return;
            }
        }
    }

    // THE MOVE SCREEN
    if (moveScreen) {
        backButton_.handleEvent(e);
        if (backButton_.clicked()) {
            backButton_.resetClick();
            playSound(game_.sounds().clickedOn);
            fightScreen = true;
            moveScreen = false;
            return;
        }

        for (int i = 0; i < 4; i++) {
            moveButtons_[i].handleEvent(e);
            updateMoveLabel(i);
            if (moveButtons_[i].clicked()) {
                moveButtons_[i].resetClick();
                playSound(game_.sounds().clickedOn);
                takeTurn(i);
                moveScreen = false;
                return;
            }
        }
    }

    // THE FIGHT SCREEN
    if (fightScreen && !inSelectionScreen) {
        fightButton_.handleEvent(e);
        pokemonButton_.handleEvent(e);
        retireButton_.handleEvent(e);
        if (fightButton_.clicked()) {
            moveScreen = true;
            fightScreen = false;
            fightButton_.resetClick();
            playSound(game_.sounds().clickedOn);
            for (int i = 0; i < 4; i++) updateMoveLabel(i);
        } else if (pokemonButton_.clicked()) {
            pokemonButton_.resetClick();
            playSound(game_.sounds().clickedOn);
            partyView_.update(player_.party);
            inSelectionScreen = true;
        } else if (retireButton_.clicked()) {
            retireButton_.resetClick();
            player_.endRun();
            respawn_();
            playSound(game_.sounds().clickedOn);
            if (transition_ == Transition::None) transition_ = Transition::FadeOut;
        }
    }
}

// Shows the move's name, or its remaining PP while the mouse is over it.
void BattleScene::updateMoveLabel(int move) {
    Button& button = moveButtons_[move];
    if (!button.contains(mouseX_, mouseY_)) {
        moveNames_[move].setText(currentPlayerPokemon->data->move[move]->name, TEXT_BLACK);
        button.resetFrame();
    } else {
        moveNames_[move].setText("PP: " + std::to_string(currentPlayerPokemon->c_pp[move]) + "/" + std::to_string(currentPlayerPokemon->data->move[move]->pp), TEXT_BLACK);
    }
}

// TURNS

void BattleScene::logOpponentFainted() {
    log_.add("The opposing " + currentOpponentPokemon->data->name + " fainted!", "OPPONENT_FAINT");

    player_.addScore(currentOpponentPokemon->data->hp);
    log_.add(player_.name() + " scored " + std::to_string(currentOpponentPokemon->data->hp) + " points!", "PLAYER_GAIN_SCORE");

    opponentFaintedPokemons++;
    if (opponentFaintedPokemons < 3) {
        log_.add(opponent_.name + " sent out " + opponent_.party[opponentFaintedPokemons].data->name + "!", "OPPONENT_NEXT_POKEMON");
    } else {
        log_.add(player_.name() + " defeated " + opponent_.name + "!", "OPPONENT_DEFEATED");
        log_.add(" ", "END_BATTLE");
    }
}

void BattleScene::logPlayerFainted() {
    log_.add(currentPlayerPokemon->data->name + " fainted!", "PLAYER_FAINT");
    playerFaintedPokemons++;
    if (playerFaintedPokemons < 3) {
        log_.add("Awaiting " + player_.name() + "'s next Pokemon...", "FORCE_OPEN_PARTY");
    } else {
        log_.add(player_.name() + " lost to " + opponent_.name + "!", "PLAYER_DEFEATED");
        log_.add(" ", "END_BATTLE");
    }
}

// The faster Pokemon attacks first (the player wins speed ties); a fainted Pokemon doesn't attack.
void BattleScene::takeTurn(int move) {
    Pokemon& mine = *currentPlayerPokemon;
    Pokemon& theirs = *currentOpponentPokemon;
    if (mine.data->speed >= theirs.data->speed) {
        if (useMove(log_, move, mine, theirs, false)) {
            logOpponentFainted();
        } else {
            int theirMove = computerChooseMove(theirs, mine);
            if (useMove(log_, theirMove, theirs, mine, true)) logPlayerFainted();
        }
    } else {
        int theirMove = computerChooseMove(theirs, mine);
        if (useMove(log_, theirMove, theirs, mine, true)) {
            logPlayerFainted();
        } else if (useMove(log_, move, mine, theirs, false)) {
            logOpponentFainted();
        }
    }
}

void BattleScene::switchPokemon(int slot) {
    Pokemon& chosen = player_.party[slot];
    if (chosen.c_hp == 0 || currentPlayerPokemon == &chosen) {
        playSound(game_.sounds().denied);
        return;
    }
    playSound(game_.sounds().aButton);
    inSelectionScreen = false;

    if (currentPlayerPokemon->c_hp == 0) { // sending out the next Pokemon after a faint
        currentPlayerPokemon = &chosen;
        log_.add(player_.name() + " sent out " + chosen.data->name + "!", "PLAYER_NEXT_POKEMON");
        playerPokeTexture_ = pokemonTexture(chosen);
        line_++;
    } else { // switching voluntarily costs the turn: the opponent gets a free attack
        log_.add(player_.name() + " withdrew " + currentPlayerPokemon->data->name + "!", "PLAYER_WITHDREW_POKEMON");
        showPHPBar = false;
        fightScreen = false;
        currentPlayerPokemon = &chosen;
        log_.add(player_.name() + " sent out " + chosen.data->name + "!", "PLAYER_NEXT_POKEMON");
        playerBeforeAttackHP = chosen.c_hp;
        int theirMove = computerChooseMove(*currentOpponentPokemon, chosen);
        if (useMove(log_, theirMove, *currentOpponentPokemon, chosen, true)) logPlayerFainted();
    }
}
