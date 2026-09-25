#include "world/Map.h"
#include "world/Camera.h"
#include "world/Direction.h"
#include "world/MapInfo.h"
#include "core/Rng.h"

#include <fstream>

namespace {
const int TILE = 64;
} // namespace

// TILE SHEET

TileSheet::TileSheet(SDL_Texture* texture) : texture_(texture) {
    int width = 0;
    SDL_QueryTexture(texture, nullptr, nullptr, &width, nullptr);
    columns_ = width / 16;
}

void TileSheet::drawTile(SDL_Renderer* renderer, int index, const SDL_Rect& dest) const {
    if (columns_ == 0) return;
    SDL_Rect src = {16 * (index % columns_), 16 * (index / columns_), 16, 16};
    SDL_RenderCopy(renderer, texture_, &src, &dest);
}

// MAP

Map::Map(Assets& assets, int mapId, bool playerHasPokemon) {
    const MapInfo& info = mapInfo(mapId);
    load(assets, assets.path(info.mapFile));
    sheet_ = TileSheet(assets.texture(info.tileset));

    if (id_ == G2_INTERIOR && !playerHasPokemon) addG2Guards(assets);
    if (isChallengeRoom(id_)) addChallengeRoomExit(assets);
}

void Map::load(Assets& assets, const std::string& path) {
    std::ifstream in(path);
    std::optional<MapData> data = parseMap(in);
    if (!data) {
        SDL_Log("Failed to load map %s", path.c_str());
        return;
    }
    id_ = data->id;
    width_ = data->width;
    height_ = data->height;
    tiles_ = std::move(data->tiles);
    properties_ = std::move(data->properties);
    overlay_ = std::move(data->overlay);
    warps_ = std::move(data->warps);
    interTiles_ = std::move(data->interTiles);

    for (const NpcData& npcData : data->npcs) {
        auto npc = std::make_unique<Npc>(npcData.x, npcData.y, npcData.facing, assets.spriteSheet(npcData.sprite), npcData.isTrainer);
        for (const std::string& sentence : npcData.preBattleDialogue) npc->addPreBattleDialogue(sentence);
        npc->trainerName = npcData.trainerName;
        npc->trainerSprite = npcData.trainerSprite;
        for (const std::string& sentence : npcData.dialogue) npc->addDialogue(sentence);
        npcs_.push_back(std::move(npc));
    }
}

void Map::addNpc(std::unique_ptr<Npc> npc) {
    if (npc->x() >= 0 && npc->y() >= 0 && npc->x() < width_ && npc->y() < height_) at(properties_, npc->x(), npc->y()) = OCCUPIED_BY_NPC;
    npcs_.push_back(std::move(npc));
}

// The G2 challenge is closed to players without Pokemon.
void Map::addG2Guards(Assets& assets) {
    for (int x : {27, 28}) {
        auto guard = std::make_unique<Npc>(x, 9, SOUTH, assets.spriteSheet("npcsprite/npcSprite6.png"));
        guard->addDialogue("I'm sorry, but you must first acquire some Pokemons before you may take the Pokemon UET Challenge.");
        guard->addDialogue("To get your Pokemons, go to the Student Button Room in the E3 Building.");
        addNpc(std::move(guard));
    }
}

// Every challenge room exits to a random next room; a guard blocks the exit until the room's
// trainer is beaten. The guard is always the last NPC (see removeLastNpc()).
void Map::addChallengeRoomExit(Assets& assets) {
    warps_.push_back({12, 7, randomInt(6, 11), 12, 22});

    auto guard = std::make_unique<Npc>(12, 8, SOUTH, assets.spriteSheet("npcsprite/npcSprite6.png"));
    guard->addDialogue("You must battle and win vs the Trainer to advance to the next room!");
    addNpc(std::move(guard));
}

int Map::property(int x, int y) const {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) return BLOCKED;
    return at(properties_, x, y);
}

void Map::draw(SDL_Renderer* renderer, const Camera& camera) const {
    for (int y = 0; y < height_; y++) {
        for (int x = 0; x < width_; x++) {
            SDL_Rect dest = {x * TILE - camera.x(), y * TILE - camera.y(), TILE, TILE};
            sheet_.drawTile(renderer, at(tiles_, x, y), dest);
        }
    }
}

void Map::drawNpcs(SDL_Renderer* renderer, const Camera& camera) {
    npcsInFront_.clear();
    for (const auto& npc : npcs_) {
        if (camera.y() + TILE * 5 < npc->y() * TILE) npcsInFront_.push_back(npc.get()); // below the player
        else npc->draw(renderer, camera.x(), camera.y());
    }
}

void Map::drawFrontNpcs(SDL_Renderer* renderer, const Camera& camera) const {
    for (const Npc* npc : npcsInFront_) npc->draw(renderer, camera.x(), camera.y());
}

void Map::drawOverlay(SDL_Renderer* renderer, const Camera& camera, const TileSheet& overlaySheet) const {
    if (overlay_.empty()) return;
    for (int y = 0; y < height_; y++) {
        for (int x = 0; x < width_; x++) {
            int tile = at(overlay_, x, y);
            if (tile == -1) continue;
            SDL_Rect dest = {x * TILE - camera.x(), y * TILE - camera.y(), TILE, TILE};
            overlaySheet.drawTile(renderer, tile, dest);
        }
    }
}

Npc* Map::npcFacing(int x, int y, int facing) {
    TilePos target = facingTile(x, y, facing);
    for (const auto& npc : npcs_) {
        if (target == TilePos{npc->x(), npc->y()} && !(target == TilePos{x, y})) return npc.get();
    }
    return nullptr;
}

const WarpTile* Map::warpFacing(int x, int y, int facing) const {
    TilePos target = facingTile(x, y, facing);
    for (const WarpTile& warp : warps_) {
        if (target == TilePos{warp.x, warp.y} && !(target == TilePos{x, y})) return &warp;
    }
    return nullptr;
}

InterTile* Map::interTileFacing(int x, int y, int facing) {
    TilePos target = facingTile(x, y, facing);
    for (InterTile& tile : interTiles_) {
        if (target == TilePos{tile.x(), tile.y()} && !(target == TilePos{x, y})) return &tile;
    }
    return nullptr;
}

void Map::removeLastNpc() {
    if (npcs_.empty()) return;
    const Npc& last = *npcs_.back();
    if (last.x() >= 0 && last.y() >= 0 && last.x() < width_ && last.y() < height_) at(properties_, last.x(), last.y()) = WALKABLE;
    npcs_.pop_back();
}
