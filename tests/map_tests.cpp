#include "world/MapData.h"
#include "world/MapInfo.h"

#include <doctest/doctest.h>

#include <fstream>
#include <sstream>
#include <string>

namespace {
std::optional<MapData> loadMap(int mapId) {
    std::ifstream in(std::string(POKEMON_ASSETS_DIR) + mapInfo(mapId).mapFile);
    REQUIRE(in.good());
    return parseMap(in);
}

bool inside(const MapData& map, int x, int y) {
    return x >= 0 && y >= 0 && x < map.width && y < map.height;
}

int property(const MapData& map, int x, int y) {
    return map.properties[y * map.width + x];
}
} // namespace

TEST_CASE("every shipped map parses consistently") {
    for (int id = 0; id < MAP_COUNT; id++) {
        CAPTURE(id);
        std::optional<MapData> map = loadMap(id);
        REQUIRE(map.has_value());
        CHECK(map->id == id);
        const size_t cells = static_cast<size_t>(map->width) * map->height;
        CHECK(map->tiles.size() == cells);
        CHECK(map->properties.size() == cells);
        CHECK(map->overlay.size() == (mapInfo(id).hasOverlay ? cells : 0));

        for (const NpcData& npc : map->npcs) {
            REQUIRE(inside(*map, npc.x, npc.y));
            CHECK(property(*map, npc.x, npc.y) == OCCUPIED_BY_NPC);
            CHECK_FALSE(npc.sprite.empty());
            CHECK(npc.sprite.rfind("res/", 0) != 0); // paths are relative to the assets folder
            if (npc.isTrainer) CHECK_FALSE(npc.preBattleDialogue.empty());
        }
        for (const WarpTile& warp : map->warps) {
            REQUIRE(inside(*map, warp.x, warp.y));
            CHECK(property(*map, warp.x, warp.y) == WARP);
            CHECK(warp.destMap >= 0);
            CHECK(warp.destMap < MAP_COUNT);
        }
        for (const InterTile& tile : map->interTiles) {
            REQUIRE(inside(*map, tile.x(), tile.y()));
            CHECK(property(*map, tile.x(), tile.y()) == INTERACTIVE);
            CHECK_FALSE(tile.dialogue.empty());
        }
    }
}

TEST_CASE("E3 exterior contents") {
    std::optional<MapData> map = loadMap(E3_EXTERIOR);
    REQUIRE(map.has_value());
    CHECK(map->width == 39);
    CHECK(map->height == 32);
    CHECK(map->npcs.size() == 3);
    CHECK(map->warps.size() == 4);
    CHECK(map->interTiles.size() == 1);
}

TEST_CASE("the Pokemon giver and a challenge room trainer") {
    std::optional<MapData> room = loadMap(BUTTON_ROOM);
    REQUIRE(room.has_value());
    REQUIRE(room->interTiles.size() == 1);
    CHECK(room->interTiles[0].x() == 13);
    CHECK(room->interTiles[0].y() == 16);
    CHECK(room->interTiles[0].dialogue.size() == 3);

    std::optional<MapData> challenge = loadMap(FIRST_CHALLENGE_ROOM);
    REQUIRE(challenge.has_value());
    REQUIRE(challenge->npcs.size() == 1);
    const NpcData& trainer = challenge->npcs[0];
    CHECK(trainer.isTrainer);
    CHECK(trainer.trainerName == "Pokemon Trainer Dust");
    CHECK(trainer.trainerSprite == "battleassets/opponentSprites/opponentSprite10.png");
    CHECK(trainer.preBattleDialogue.size() == 5);
}

TEST_CASE("inspecting a tile steps through its lines, then ends") {
    InterTile tile(1, 1);
    tile.dialogue = {"first", "second"};
    CHECK(tile.talk());
    CHECK(tile.currentSentence() == "first");
    CHECK(tile.talk());
    CHECK(tile.currentSentence() == "second");
    CHECK_FALSE(tile.talk());
    CHECK(tile.currentSentence() == "");
}

TEST_CASE("an invalid map header is rejected") {
    std::istringstream empty("");
    CHECK_FALSE(parseMap(empty).has_value());
    std::istringstream zeroSize("1 0 0");
    CHECK_FALSE(parseMap(zeroSize).has_value());
}
