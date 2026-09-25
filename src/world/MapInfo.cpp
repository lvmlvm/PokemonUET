#include "world/MapInfo.h"

namespace {
const MapInfo MAPS[MAP_COUNT] = {
    {"map/g2.map", "tileset/g2o_tiles.png", "music/g2o_theme.mp3", 8.85, false},
    {"map/e3.map", "tileset/e3o_tiles.png", "music/e3o_theme.mp3", 30.555, false},
    {"map/e3i.map", "tileset/e3i_tiles.png", "music/e3i_theme.mp3", 0.77, true},
    {"map/g2i.map", "tileset/g2i_tiles.png", "music/e3i_theme.mp3", 0.77, true},
    {"map/e3i_2.map", "tileset/e3i_2_tiles.png", "music/e3i_theme.mp3", 0.77, true},
    {"map/chal5.map", "tileset/chal5_tiles.png", "music/e3i_theme.mp3", 0.77, false},
    {"map/chal6.map", "tileset/chal6_tiles.png", "music/e3i_theme.mp3", 0.77, false},
    {"map/chal7.map", "tileset/chal7_tiles.png", "music/e3i_theme.mp3", 0.77, false},
    {"map/chal8.map", "tileset/chal8_tiles.png", "music/e3i_theme.mp3", 0.77, false},
    {"map/chal9.map", "tileset/chal9_tiles.png", "music/e3i_theme.mp3", 0.77, false},
    {"map/chal10.map", "tileset/chal10_tiles.png", "music/e3i_theme.mp3", 0.77, false},
    {"map/chal11.map", "tileset/chal11_tiles.png", "music/e3i_theme.mp3", 0.77, false},
};
} // namespace

const MapInfo& mapInfo(int mapId) {
    if (mapId < 0 || mapId >= MAP_COUNT) mapId = E3_EXTERIOR;
    return MAPS[mapId];
}

bool isChallengeRoom(int mapId) {
    return mapId >= FIRST_CHALLENGE_ROOM && mapId <= LAST_CHALLENGE_ROOM;
}
