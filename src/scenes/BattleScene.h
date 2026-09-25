#pragma once

#include "battle/BattleEngine.h"
#include "core/Game.h"
#include "ui/Button.h"
#include "ui/PartyView.h"
#include "ui/TextLabel.h"

#include <array>
#include <functional>
#include <string>

// A trainer battle, as seen on screen. The BattleEngine decides what happens; each player
// action produces a list of BattleEvents that the scene plays back one line per X press,
// animating each event.
class BattleScene : public Scene {
public:
    // `respawn` sends the player back to the E3 entrance (after losing or retiring).
    BattleScene(Game& game, Trainer& opponent, std::function<void()> respawn);

    void handleEvent(const SDL_Event& event) override;
    void frame() override;

private:
    enum class Transition { FadeIn, None, FadeOut, Done };

    void draw();
    void drawIntro();
    void animateCurrentAction();
    void takeTurn(int move);
    void switchPokemon(int slot);
    void addEvents(const std::vector<BattleEvent>& events);
    void updateMoveLabel(int move);
    bool eventIs(unsigned int i, BattleEvent::Kind kind) const;
    const std::string& lineText(unsigned int i) const;
    int playerHpWidth(int hp) const;
    int opponentHpWidth() const;
    TexturePtr pokemonTexture(const Pokemon& pokemon);

    Game& game_;
    SDL_Renderer* renderer_;
    Player& player_;
    std::function<void()> respawn_;
    Transition transition_ = Transition::FadeIn;

    BattleEngine engine_;
    // The opponent's Pokemon on screen. It only changes once the next one's send-out plays,
    // while the engine moves on as soon as the previous one faints.
    Pokemon* shownOpponent_;
    std::array<std::string, 3> intro_;  // shown before the first turn
    std::vector<BattleEvent> log_;      // the current turn
    unsigned int line_ = 0;             // the line being shown

    // Textures
    SDL_Texture* background_;
    SDL_Texture* circle_;
    SDL_Texture* grayBox_;
    SDL_Texture* playerHPBar_;
    SDL_Texture* oppoHPBar_;
    SDL_Texture* hpColor_;
    SDL_Texture* playerTrainerTexture_;
    SDL_Texture* opponentTrainerTexture_;
    TexturePtr playerPokeTexture_;   // own copies: their alpha blinks when hit
    TexturePtr opponentPokeTexture_;

    // Layout and animation state
    SDL_Rect playerHPRect = {548, 388, 284, 95}, oppoHPRect = {0, 66, 273, 78};
    SDL_Rect currPlayHP, currOppoHP;
    SDL_Rect playerCircle = {832, 450, 320, 100}, opponentCircle = {-320, 200, 320, 100};
    SDL_Rect grayBoxRect = {0, 704, 832, 204};
    SDL_Rect playerSpriteBox = {832, 260, 240, 240}, opponentSpriteBox = {-320, 20, 240, 240};
    SDL_Rect playerPokeRect = {-240, 300, 240, 240}, oppoPokeRect = {832, 20, 240, 240};
    SDL_Rect halfPlayerPokeRect = {0, 0, 96, 96}, halfOppoPokeRect = {96, 0, 96, 96}; // back / front sprite
    std::array<SDL_Rect, 4> playerTextureFrames;
    int currentPlayerFrame = 0;
    int playerBeforeAttackHP = 0;
    double pCirX = 0, oCirX = 0, playerPokemonMoveAnim = 0, opponentPokemonMoveAnim = 0, PokemonTransparency = 0;

    // Screen state
    bool inAnim0 = true, fightScreen = false, moveScreen = false, inSelectionScreen = false,
         showPHPBar = false, showOHPBar = false, startingBattle = true;

    TextLabel dialogueText_, playerPokeName_, oppoPokeName_;
    std::array<TextLabel, 4> moveNames_;
    Button fightButton_, pokemonButton_, retireButton_, backButton_;
    std::array<Button, 4> moveButtons_;
    PartyView partyView_;
    int mouseX_ = 0, mouseY_ = 0; // last known mouse position, from events

    Mix_Chunk *effectiveSFX, *notEffectiveSFX, *superEffectiveSFX, *pokemonFainted;
};
