#pragma once

// Facing directions, in sprite-sheet order.
enum Direction { SOUTH = 0, EAST = 1, NORTH = 2, WEST = 3 };

struct TilePos {
    int x, y;
    bool operator==(const TilePos& other) const { return x == other.x && y == other.y; }
};

// The tile directly in front of (x, y) when facing `facing`.
inline TilePos facingTile(int x, int y, int facing) {
    switch (facing) {
        case SOUTH: return {x, y + 1};
        case EAST: return {x + 1, y};
        case NORTH: return {x, y - 1};
        case WEST: return {x - 1, y};
        default: return {x, y}; // invalid facing: matches nothing adjacent
    }
}

inline int oppositeDirection(int facing) {
    return (facing + 2) % 4;
}
