#include "world/Map.h"
#include "world/Camera.h"
#include "world/Direction.h"
#include "world/MapInfo.h"
#include "core/Rng.h"

#include <fstream>

namespace {
const int TILE = 64;

// Asset paths inside .map files are written relative to the game folder ("res/...").
std::string assetPath(const std::string& path) {
    const std::string prefix = "res/";
    return path.compare(0, prefix.size(), prefix) == 0 ? path.substr(prefix.size()) : path;
}

bool readLine(std::istream& in, std::string& line) {
    if (!std::getline(in, line)) return false;
    if (!line.empty() && line.back() == '\r') line.pop_back();
    return true;
}

// Reads lines up to (not including) `endMarker`, skipping empty lines.
std::vector<std::string> readBlock(std::istream& in, const std::string& endMarker) {
    std::vector<std::string> lines;
    std::string line;
    while (readLine(in, line) && line != endMarker) {
        if (!line.empty()) lines.push_back(line);
    }
    return lines;
}
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

// File format: id, height, width, the tile layer, the property layer and (for maps with one)
// the overlay layer, each followed by an end marker; then NPCs, warp tiles and interactive
// tiles, each entry introduced by a *_NEXT_* marker and each list ended by a *_STOP marker.
void Map::load(Assets& assets, const std::string& path) {
    std::ifstream in(path);
    in >> id_ >> height_ >> width_;
    if (!in || height_ <= 0 || width_ <= 0) {
        SDL_Log("Failed to load map %s", path.c_str());
        width_ = height_ = 0;
        return;
    }

    const size_t cells = static_cast<size_t>(width_) * height_;
    std::string marker;
    auto readLayer = [&](std::vector<int>& layer) {
        layer.assign(cells, 0);
        for (int& value : layer) in >> value;
        in >> marker; // end-of-layer marker
    };
    readLayer(tiles_);
    readLayer(properties_);
    if (mapInfo(id_).hasOverlay) readLayer(overlay_);

    while (in >> marker && marker == "MAP_NEXT_NPC") {
        int x, y, facing;
        bool isTrainer;
        std::string sprite;
        in >> x >> y >> facing >> isTrainer >> sprite;

        auto npc = std::make_unique<Npc>(x, y, facing, assets.spriteSheet(assetPath(sprite)), isTrainer);
        if (isTrainer) {
            for (const std::string& sentence : readBlock(in, "NPC_PREBATTLE_END")) npc->addPreBattleDialogue(sentence);
            readLine(in, npc->trainerName);
            std::string trainerSprite;
            readLine(in, trainerSprite);
            npc->trainerSprite = assetPath(trainerSprite);
        }
        for (const std::string& sentence : readBlock(in, "NPC_DIALOGUE_END")) npc->addDialogue(sentence);

        if (x >= 0 && y >= 0 && x < width_ && y < height_) at(properties_, x, y) = OCCUPIED_BY_NPC;
        npcs_.push_back(std::move(npc));
    }

    while (in >> marker && marker == "WARP_NEXT_TILE") {
        WarpTile warp{};
        in >> warp.x >> warp.y >> warp.destMap >> warp.destX >> warp.destY;
        if (warp.x >= 0 && warp.y >= 0 && warp.x < width_ && warp.y < height_) at(properties_, warp.x, warp.y) = WARP;
        warps_.push_back(warp);
    }

    while (in >> marker && marker == "INTER_NEXT_TILE") {
        int x, y;
        in >> x >> y;
        InterTile tile(x, y);
        tile.dialogue = readBlock(in, "INTER_DIALOGUE_END");
        if (x >= 0 && y >= 0 && x < width_ && y < height_) at(properties_, x, y) = INTERACTIVE;
        interTiles_.push_back(tile);
    }
}

// The G2 challenge is closed to players without Pokemon.
void Map::addG2Guards(Assets& assets) {
    for (int x : {27, 28}) {
        auto guard = std::make_unique<Npc>(x, 9, SOUTH, assets.spriteSheet("npcsprite/npcSprite6.png"));
        guard->addDialogue("I'm sorry, but you must first acquire some Pokemons before you may take the Pokemon UET Challenge.");
        guard->addDialogue("To get your Pokemons, go to the Student Button Room in the E3 Building.");
        npcs_.push_back(std::move(guard));
        at(properties_, x, 9) = OCCUPIED_BY_NPC;
    }
}

// Every challenge room exits to a random next room; a guard blocks the exit until the room's
// trainer is beaten. The guard is always the last NPC (see removeLastNpc()).
void Map::addChallengeRoomExit(Assets& assets) {
    warps_.push_back({12, 7, randomInt(6, 11), 12, 22});

    auto guard = std::make_unique<Npc>(12, 8, SOUTH, assets.spriteSheet("npcsprite/npcSprite6.png"));
    guard->addDialogue("You must battle and win vs the Trainer to advance to the next room!");
    npcs_.push_back(std::move(guard));
    at(properties_, 12, 8) = OCCUPIED_BY_NPC;
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
