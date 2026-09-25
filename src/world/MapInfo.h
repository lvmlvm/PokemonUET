#pragma once

// Static description of each map, indexed by map ID:
// 0 = G2 exterior, 1 = E3 exterior, 2 = E3 interior, 3 = G2 interior,
// 4 = Student Button Room, 5..11 = challenge rooms.
struct MapInfo {
    const char* mapFile;
    const char* tileset;
    const char* theme;
    double themeRepeatPoint; // seconds; later loops of the theme restart here
    bool hasOverlay;         // has a layer drawn above the player
};

const int MAP_COUNT = 12;
const int E3_EXTERIOR = 1;
const int G2_INTERIOR = 3;
const int BUTTON_ROOM = 4;
const int FIRST_CHALLENGE_ROOM = 5;
const int LAST_CHALLENGE_ROOM = 11;

const MapInfo& mapInfo(int mapId);
bool isChallengeRoom(int mapId);
