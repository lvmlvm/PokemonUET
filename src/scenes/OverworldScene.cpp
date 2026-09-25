#include "scenes/OverworldScene.h"
#include "ui/Colors.h"
#include "scenes/BattleScene.h"
#include "core/Rng.h"
#include "world/Direction.h"
#include "world/MapInfo.h"

namespace {
const SDL_Rect DIALOGUE_BOX = {10, 515, 812, 179};
const int MAP_FADE_STEP = 17, BATTLE_FADE_STEP = 5;
const TilePos POKEMON_GIVER = {13, 16}; // in the Student Button Room

bool isMovementKey(SDL_Keycode key) {
    return key == SDLK_w || key == SDLK_a || key == SDLK_s || key == SDLK_d;
}
} // namespace

OverworldScene::OverworldScene(Game& game)
    : game_(game),
      renderer_(game.renderer()),
      player_(game.player()),
      overlaySheet_(game.assets().texture("tileset/overlaying_tiles.png")),
      dialogueBox_(game.assets().texture("otherassets/dialoguebox.png")),
      dialogueText_(renderer_, game.assets().font(38)),
      menu_(game) {
    loadMap(player_.currentMap());
    camera_.centerOn(player_.x(), player_.y());
    const char* sheet = player_.gender() == 0 ? "playersprite/malesprite.png" : "playersprite/femalesprite.png";
    sprite_ = PlayerSprite(game.assets().spriteSheet(sheet));

    const MapInfo& info = mapInfo(player_.currentMap());
    game.music().load(game.assets().path(info.theme), info.themeRepeatPoint);
}

void OverworldScene::loadMap(int mapId) {
    map_ = std::make_unique<Map>(game_.assets(), mapId, player_.hasPokemon());
}

void OverworldScene::respawnAtE3() {
    loadMap(E3_EXTERIOR);
    player_.setPosition(20, 10, E3_EXTERIOR);
    camera_.centerOn(player_.x(), player_.y());
    player_.setFacing(SOUTH);
}

void OverworldScene::onQuit() {
    game_.savePlayer();
}

void OverworldScene::onResume() {
    transition_ = Transition::BattleIn;
}

// INPUT

void OverworldScene::handleEvent(const SDL_Event& e) {
    if ((e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) && e.key.keysym.sym == SDLK_z) running_ = e.type == SDL_KEYDOWN;

    const bool leavingMap = transition_ == Transition::WarpOut || transition_ == Transition::BattleOut;
    const SDL_Keycode key = e.key.keysym.sym;

    if (e.type == SDL_KEYDOWN && key == SDLK_v && e.key.repeat == 0 && !leavingMap) { // OPEN THE MENU
        menu_.open();
        playSound(game_.sounds().startMenu);
    } else if (e.type == SDL_KEYDOWN && key == SDLK_x && e.key.repeat == 0 && !menu_.isOpen() && !leavingMap) { // TALK / INSPECT
        interact();
    } else if (e.type == SDL_KEYDOWN && !camera_.isMoving() && e.key.repeat == 0 && !inDialogue_ && !menu_.isOpen()
               && transition_ != Transition::WarpOut && transition_ != Transition::WarpIn) { // TURN AND START WALKING
        switch (key) {
            case SDLK_s: player_.setFacing(SOUTH); break;
            case SDLK_d: player_.setFacing(EAST); break;
            case SDLK_w: player_.setFacing(NORTH); break;
            case SDLK_a: player_.setFacing(WEST); break;
            default: break;
        }
        if (isMovementKey(key) && map_->warpFacing(player_.x(), player_.y(), player_.facing()) != nullptr) {
            transition_ = Transition::WarpOut;
        } else {
            camera_.beginMovement(key, player_.x(), player_.y(), *map_);
        }
    } else if (e.type == SDL_KEYUP && camera_.isMoving() && e.key.repeat == 0) { // STOP WALKING (at the next tile)
        camera_.stopMovement(key);
    }

    if (menu_.isOpen()) menu_.handleEvent(e);
}

void OverworldScene::interact() {
    if (Npc* npc = map_->npcFacing(player_.x(), player_.y(), player_.facing())) {
        Npc::Talk result = npc->talk(player_.facing());
        inDialogue_ = result == Npc::Talk::Talking;
        if (result == Npc::Talk::StartBattle) transition_ = Transition::BattleOut;
        playSound(game_.sounds().aButton);
    } else if (InterTile* tile = map_->interTileFacing(player_.x(), player_.y(), player_.facing())) {
        inDialogue_ = tile->talk();
        playSound(game_.sounds().aButton);
    }
}

// FRAME

void OverworldScene::frame() {
    frameX_ = player_.x();
    frameY_ = player_.y();

    if (running_) camera_.speedUp();
    else camera_.slowDown();

    if (camera_.isFinishing()) {
        camera_.finishMovement();
    } else {
        camera_.move(frameX_, frameY_, *map_);
        camera_.clampToMap(map_->width(), map_->height());
    }
    player_.setPosition(camera_.playerTileX(), camera_.playerTileY(), map_->id());

    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);

    map_->draw(renderer_, camera_);
    map_->drawNpcs(renderer_, camera_);
    if (!camera_.isMoving()) sprite_.drawStanding(renderer_, player_.facing());
    else if (running_) sprite_.drawRunning(renderer_, player_.facing());
    else sprite_.drawWalking(renderer_, player_.facing());
    map_->drawFrontNpcs(renderer_, camera_);
    if (mapInfo(player_.currentMap()).hasOverlay) map_->drawOverlay(renderer_, camera_, overlaySheet_);

    if (inDialogue_) drawDialogue();
    if (menu_.isOpen()) menu_.draw();

    updateTransitions();

    if (transition_ != Transition::WarpOut && transition_ != Transition::BattleIn) game_.music().play();
}

void OverworldScene::drawDialogue() {
    SDL_RenderCopy(renderer_, dialogueBox_, nullptr, &DIALOGUE_BOX);
    if (const Npc* npc = map_->npcFacing(frameX_, frameY_, player_.facing())) {
        dialogueText_.setText(npc->currentSentence(), TEXT_BLACK);
        dialogueText_.draw(48, 547);
    } else if (InterTile* tile = map_->interTileFacing(frameX_, frameY_, player_.facing())) {
        dialogueText_.setText(tile->currentSentence(), TEXT_BLACK);
        dialogueText_.draw(48, 547);
        if (player_.currentMap() == BUTTON_ROOM && TilePos{tile->x(), tile->y()} == POKEMON_GIVER) updatePokemonGiver(*tile);
    }
}

// The Student Button Room's slot machine rolls three random Pokemon, then announces them.
void OverworldScene::updatePokemonGiver(InterTile& tile) {
    if (tile.sentenceNumber() == 2) { // rolling (re-rolls every frame until the next line)
        for (Pokemon& pokemon : player_.party) pokemon = Pokemon(randomInt(1, psize - 1));
    } else if (tile.sentenceNumber() == 1 && tile.dialogue.size() > 3) { // talking again: drop the last announcement
        tile.dialogue.pop_back();
    } else if (tile.sentenceNumber() == 3 && tile.dialogue.size() < 4) {
        tile.dialogue.push_back(player_.name() + " received " + player_.party[0].data->name + ", "
                                + player_.party[1].data->name + " and " + player_.party[2].data->name + "!");
    }
}

// Runs the active fade, in this order; a warp fades out and starts fading back in on the same frame.
void OverworldScene::updateTransitions() {
    Fade& fade = game_.fade();

    if (transition_ == Transition::FadeInFromTitle) {
        if (fade.in(MAP_FADE_STEP)) transition_ = Transition::None;
        fade.draw(renderer_);
    }

    if (transition_ == Transition::WarpOut) {
        if (fade.out(MAP_FADE_STEP)) warp();
        else if (fade.alpha() == 51) playSound(game_.sounds().changeMap);
        fade.draw(renderer_);
    }

    if (transition_ == Transition::WarpIn) {
        if (fade.in(MAP_FADE_STEP)) {
            transition_ = Transition::None;
            game_.savePlayer();
        }
        fade.draw(renderer_);
    }

    if (transition_ == Transition::BattleOut) {
        if (!transitionMusicLoaded_) {
            game_.music().resetChord();
            game_.music().load(game_.assets().path("music/battleMusic.mp3"), 19.9);
            transitionMusicLoaded_ = true;
        }
        if (fade.out(BATTLE_FADE_STEP)) {
            transitionMusicLoaded_ = false;
            startBattle();
        }
        fade.draw(renderer_);
    }

    if (transition_ == Transition::BattleIn) {
        if (!transitionMusicLoaded_) {
            const MapInfo& info = mapInfo(player_.currentMap());
            game_.music().resetChord();
            game_.music().load(game_.assets().path(info.theme), info.themeRepeatPoint);
            transitionMusicLoaded_ = true;
        }
        if (fade.in(BATTLE_FADE_STEP)) {
            transitionMusicLoaded_ = false;
            transition_ = Transition::None;
            player_.healParty();
        }
        fade.draw(renderer_);
    }
}

void OverworldScene::warp() {
    transition_ = Transition::WarpIn;
    const WarpTile* warp = map_->warpFacing(frameX_, frameY_, player_.facing());
    if (warp == nullptr) return; // should not happen: the warp started while facing a warp tile
    const WarpTile destination = *warp; // copied: the current map is about to be replaced

    const MapInfo& from = mapInfo(player_.currentMap());
    const MapInfo& to = mapInfo(destination.destMap);
    if (std::string(from.theme) != to.theme) {
        Mix_FadeOutMusic(500);
        game_.music().load(game_.assets().path(to.theme), to.themeRepeatPoint);
        game_.music().resetChord();
    }

    loadMap(destination.destMap);
    player_.setPosition(destination.destX, destination.destY, destination.destMap);
    camera_.centerOn(player_.x(), player_.y());
}

void OverworldScene::startBattle() {
    Npc* trainer = map_->npcFacing(frameX_, frameY_, player_.facing());
    if (trainer == nullptr) { // should not happen: the battle started while facing the trainer
        transition_ = Transition::FadeInFromTitle;
        return;
    }
    Trainer& opponent = game_.opponent();
    opponent.name = trainer->trainerName;
    opponent.battleSpritePath = trainer->trainerSprite;
    trainer->markBattled();
    // Beating a challenge room's trainer lets the player past its exit guard (the last NPC).
    if (isChallengeRoom(player_.currentMap())) map_->removeLastNpc();

    for (Pokemon& pokemon : opponent.party) pokemon = Pokemon(randomInt(1, psize - 1));

    game_.fade().setAlpha(255);
    transition_ = Transition::None;
    game_.pushScene(std::make_unique<BattleScene>(game_, opponent, [this] { respawnAtE3(); }));
}
