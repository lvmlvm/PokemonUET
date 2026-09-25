#pragma once

#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

// The contents of a .map file, without any rendering resources (no SDL dependency).

struct WarpTile {
    int x, y;
    int destMap, destX, destY;
};

// A tile the player can inspect (signs, machines, ...).
class InterTile {
public:
    InterTile(int x, int y) : x_(x), y_(y) {}

    bool talk(); // advances the dialogue; false once it is over
    std::string currentSentence() const;
    unsigned int sentenceNumber() const { return cursor_; }

    int x() const { return x_; }
    int y() const { return y_; }

    std::vector<std::string> dialogue;

private:
    int x_, y_;
    unsigned int cursor_ = 0; // 1-based index of the sentence being shown, 0 = not talking
};

struct NpcData {
    int x = 0, y = 0, facing = 0;
    bool isTrainer = false;
    std::string sprite; // asset path
    std::vector<std::string> preBattleDialogue; // trainers only
    std::string trainerName, trainerSprite;     // trainers only
    std::vector<std::string> dialogue;
};

// Collision values of the property layer.
enum TileProperty { WALKABLE = 0, BLOCKED = 1, WARP = 2, INTERACTIVE = 3, OCCUPIED_BY_NPC = 4 };

struct MapData {
    int id = -1;
    int width = 0, height = 0;
    std::vector<int> tiles;      // tile sheet index per cell, row by row
    std::vector<int> properties; // TileProperty per cell; NPC, warp and interactive cells are marked
    std::vector<int> overlay;    // drawn above the player (-1 = nothing); empty if the map has none
    std::vector<NpcData> npcs;
    std::vector<WarpTile> warps;
    std::vector<InterTile> interTiles;
};

// File format: id, height, width, the tile layer, the property layer and (for maps whose MapInfo
// says so) the overlay layer, each followed by an end marker; then NPCs, warp tiles and
// interactive tiles, each entry introduced by a *_NEXT_* marker and each list ended by a *_STOP
// marker. Returns nothing if the header is invalid.
std::optional<MapData> parseMap(std::istream& in);
