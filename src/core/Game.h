#pragma once

#include "core/Assets.h"
#include "core/Fade.h"
#include "core/Music.h"
#include "core/Scene.h"
#include "core/SdlContext.h"
#include "world/Player.h"

#include <memory>
#include <vector>

// Sound effects shared by several scenes.
struct Sounds {
    explicit Sounds(Assets& assets);

    Mix_Chunk* changeMap;
    Mix_Chunk* aButton;
    Mix_Chunk* gameSaved;
    Mix_Chunk* startMenu;
    Mix_Chunk* denied;
    Mix_Chunk* clickedOn;
};

void playSound(Mix_Chunk* sound);

// Owns the SDL context, the shared services and game state, and the scene stack.
class Game {
public:
    static const int SCREEN_WIDTH = 832, SCREEN_HEIGHT = 704;

    Game();
    void run();

    SDL_Renderer* renderer() const { return sdl_.renderer(); }
    Assets& assets() { return assets_; }
    Fade& fade() { return fade_; }
    Music& music() { return music_; }
    const Sounds& sounds() const { return sounds_; }
    Player& player() { return player_; }
    Trainer& opponent() { return opponent_; }
    bool hasSaveFile() const { return hasSaveFile_; }
    bool savePlayer();

    // Scene changes take effect at the start of the next frame.
    void replaceScene(std::unique_ptr<Scene> scene);
    void pushScene(std::unique_ptr<Scene> scene);
    void popScene();
    void quit() { quit_ = true; }

private:
    void loadSave();
    void applySceneChanges();

    SdlContext sdl_; // first member: SDL outlives everything below
    Assets assets_;
    Fade fade_;
    Music music_;
    Sounds sounds_;
    Trainer opponent_; // rolled once at startup, re-rolled for every battle
    Player player_;
    std::string saveFile_;
    bool hasSaveFile_ = false;
    bool quit_ = false;

    std::vector<std::unique_ptr<Scene>> scenes_;
    std::unique_ptr<Scene> pendingScene_;
    enum class Change { None, Replace, Push, Pop } pendingChange_ = Change::None;
};
