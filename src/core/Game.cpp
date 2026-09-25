#include "core/Game.h"
#include "core/Replay.h"
#include "scenes/TitleScene.h"

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
    hasSaveFile_ = player_.load(SAVE_FILE);
    scenes_.push_back(std::make_unique<TitleScene>(*this));
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
        replay::delay(1000 / 60);
    }
}
