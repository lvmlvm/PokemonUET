#pragma once

#include "core/Game.h"
#include "ui/GameMenu.h"
#include "ui/TextLabel.h"
#include "world/Camera.h"
#include "world/Map.h"
#include "world/PlayerSprite.h"

#include <memory>

// Walking around the maps: movement, talking to NPCs and signs, warps between maps, the menu,
// and starting trainer battles.
class OverworldScene : public Scene {
public:
    explicit OverworldScene(Game& game);

    void handleEvent(const SDL_Event& event) override;
    void frame() override;
    void onQuit() override;
    void onResume() override; // back from a battle

private:
    enum class Transition {
        None,
        FadeInFromTitle,
        WarpOut, WarpIn,       // between maps
        BattleOut, BattleIn,   // to and from a battle
    };

    void loadMap(int mapId);
    void respawnAtE3();
    void interact();
    void drawDialogue();
    void updatePokemonGiver(InterTile& tile);
    void updateTransitions();
    void warp();
    void startBattle();

    Game& game_;
    SDL_Renderer* renderer_;
    Player& player_;
    std::unique_ptr<Map> map_;
    Camera camera_;
    PlayerSprite sprite_;
    TileSheet overlaySheet_;
    SDL_Texture* dialogueBox_;
    TextLabel dialogueText_;
    GameMenu menu_;

    Transition transition_ = Transition::FadeInFromTitle;
    bool transitionMusicLoaded_ = false;
    bool inDialogue_ = false;
    bool rolledPokemon_ = false; // the Pokemon giver already rolled during this line
    bool running_ = false; // Z held
    int frameX_ = 0, frameY_ = 0; // player position at the start of the frame
};
