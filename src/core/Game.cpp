#include "core/Game.h"
#include "core/Replay.h"
#include "save/SaveGame.h"
#include "scenes/TitleScene.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>
#include <vector>

namespace {
// Where saves go: POKEMON_SAVE_DIR if set (tests), otherwise the per-user app data folder
// (e.g. ~/Library/Application Support/UET/PokemonUET/ on macOS, %APPDATA%\UET\PokemonUET\ on Windows).
std::string saveDirectory() {
    if (const char* dir = std::getenv("POKEMON_SAVE_DIR")) {
        std::error_code ignored;
        std::filesystem::create_directories(dir, ignored);
        std::string path = dir;
        if (!path.empty() && path.back() != '/' && path.back() != '\\') path += '/';
        return path;
    }
    if (char* pref = SDL_GetPrefPath("UET", "PokemonUET")) {
        std::string path = pref;
        SDL_free(pref);
        return path;
    }
    return "data/";
}

// Saves from before the save folder moved: data/player.sav next to the game.
std::vector<std::string> legacySaveFiles() {
    std::vector<std::string> paths = {"data/player.sav"};
    if (char* base = SDL_GetBasePath()) {
        paths.push_back(std::string(base) + "data/player.sav");
        SDL_free(base);
    }
    return paths;
}
} // namespace

Sounds::Sounds(Assets& assets)
    : changeMap(assets.sound("sfx/change_map.wav")),
      aButton(assets.sound("sfx/a_button.wav")),
      gameSaved(assets.sound("sfx/game_saved.wav")),
      startMenu(assets.sound("sfx/start_menu.wav")),
      denied(assets.sound("sfx/denied.wav")),
      clickedOn(assets.sound("sfx/clicked_on_sound.wav")) {}

void playSound(Mix_Chunk* sound) {
    if (sound != nullptr) Mix_PlayChannel(-1, sound, 0);
}

Game::Game()
    : sdl_("Pokémon UET", SCREEN_WIDTH, SCREEN_HEIGHT),
      assets_(sdl_.renderer(), Assets::findRoot()),
      fade_(assets_.loadTexture("otherassets/blacktransition.png")),
      sounds_(assets_) {
    replay::load();
    loadSave();
    scenes_.push_back(std::make_unique<TitleScene>(*this));
}

void Game::loadSave() {
    saveFile_ = saveDirectory() + "player.sav";

    if (std::filesystem::exists(saveFile_)) {
        if (std::optional<Player> saved = SaveGame::readFile(saveFile_)) {
            player_ = *saved;
            hasSaveFile_ = true;
        } else {
            std::cout << "Save file is corrupted! Default player config loaded instead!\n";
        }
        return;
    }

    for (const std::string& legacy : legacySaveFiles()) {
        if (std::optional<Player> saved = SaveGame::readFile(legacy)) {
            player_ = *saved;
            hasSaveFile_ = true;
            std::cout << "Imported save file " << legacy << " into " << saveFile_ << '\n';
            savePlayer();
            return;
        }
    }
    std::cout << "No save file detected! Default player config loaded instead!\n";
}

bool Game::savePlayer() {
    bool saved = SaveGame::writeFile(saveFile_, player_);
    if (!saved) SDL_Log("Failed to save the game to %s", saveFile_.c_str());
    return saved;
}

void Game::replaceScene(std::unique_ptr<Scene> scene) {
    pendingScene_ = std::move(scene);
    pendingChange_ = Change::Replace;
}

void Game::pushScene(std::unique_ptr<Scene> scene) {
    pendingScene_ = std::move(scene);
    pendingChange_ = Change::Push;
}

void Game::popScene() {
    pendingChange_ = Change::Pop;
}

void Game::applySceneChanges() {
    switch (pendingChange_) {
        case Change::None:
            return;
        case Change::Replace:
            scenes_.clear();
            scenes_.push_back(std::move(pendingScene_));
            break;
        case Change::Push:
            scenes_.push_back(std::move(pendingScene_));
            break;
        case Change::Pop:
            scenes_.pop_back();
            if (!scenes_.empty()) scenes_.back()->onResume();
            break;
    }
    pendingChange_ = Change::None;
    if (scenes_.empty()) quit_ = true;
}

void Game::run() {
    while (!quit_) {
        const Uint64 frameStart = SDL_GetPerformanceCounter();
        applySceneChanges();
        if (quit_) break;
        Scene& scene = *scenes_.back();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                scene.onQuit();
                quit_ = true;
            } else {
                scene.handleEvent(event);
            }
        }

        scene.frame();

        replay::beforePresent(renderer());
        SDL_RenderPresent(renderer());
        waitForNextFrame(frameStart);
    }
}

// The game logic advances one step per frame, so frames are paced at a fixed rate
// (deliberately not vsync, which would tie the game's speed to the display's refresh rate).
void Game::waitForNextFrame(Uint64 frameStart) {
    const double speed = replay::speed();
    if (speed <= 0) return;
    const Uint64 frequency = SDL_GetPerformanceFrequency();
    const Uint64 frameTicks = static_cast<Uint64>(frequency / (FRAMES_PER_SECOND * speed));
    const Uint64 elapsed = SDL_GetPerformanceCounter() - frameStart;
    if (elapsed < frameTicks) SDL_Delay(static_cast<Uint32>((frameTicks - elapsed) * 1000 / frequency));
}
