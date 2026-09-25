#pragma once

#include "core/Assets.h"
#include "world/Npc.h"

#include <memory>
#include <string>
#include <vector>

class Camera;

// A tile sheet of 16x16 tiles, drawn scaled to 64x64.
class TileSheet {
public:
    TileSheet() = default;
    explicit TileSheet(SDL_Texture* texture);

    void drawTile(SDL_Renderer* renderer, int index, const SDL_Rect& dest) const;

private:
    SDL_Texture* texture_ = nullptr;
    int columns_ = 0;
};

// Collision values of the property layer.
enum TileProperty { WALKABLE = 0, BLOCKED = 1, WARP = 2, INTERACTIVE = 3, OCCUPIED_BY_NPC = 4 };

class Map {
public:
    // `playerHasPokemon` decides whether the guards blocking the G2 challenge stay in place.
    Map(Assets& assets, int mapId, bool playerHasPokemon);

    int id() const { return id_; }
    int width() const { return width_; }
    int height() const { return height_; }
    int property(int x, int y) const; // BLOCKED outside the map

    void draw(SDL_Renderer* renderer, const Camera& camera) const;
    void drawNpcs(SDL_Renderer* renderer, const Camera& camera);      // NPCs behind the player
    void drawFrontNpcs(SDL_Renderer* renderer, const Camera& camera) const; // ... and in front
    void drawOverlay(SDL_Renderer* renderer, const Camera& camera, const TileSheet& overlaySheet) const;

    // What the player standing at (x, y) and facing `facing` is looking at, or null.
    Npc* npcFacing(int x, int y, int facing);
    const WarpTile* warpFacing(int x, int y, int facing) const;
    InterTile* interTileFacing(int x, int y, int facing);

    void removeLastNpc();

private:
    void load(Assets& assets, const std::string& path);
    void addG2Guards(Assets& assets);
    void addChallengeRoomExit(Assets& assets);
    int& at(std::vector<int>& layer, int x, int y) { return layer[y * width_ + x]; }
    int at(const std::vector<int>& layer, int x, int y) const { return layer[y * width_ + x]; }

    int id_ = -1;
    int width_ = 0, height_ = 0;
    std::vector<int> tiles_;
    std::vector<int> properties_;
    std::vector<int> overlay_; // empty when the map has no overlay layer
    TileSheet sheet_;
    std::vector<std::unique_ptr<Npc>> npcs_;
    std::vector<Npc*> npcsInFront_; // rebuilt every frame by drawNpcs()
    std::vector<WarpTile> warps_;
    std::vector<InterTile> interTiles_;
};
