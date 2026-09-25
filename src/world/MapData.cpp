#include "world/MapData.h"
#include "world/MapInfo.h"

#include <istream>

namespace {
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

bool InterTile::talk() {
    if (cursor_ < dialogue.size()) {
        cursor_++;
        return true;
    }
    cursor_ = 0;
    return false;
}

std::string InterTile::currentSentence() const {
    if (cursor_ == 0 || cursor_ > dialogue.size()) return "";
    return dialogue[cursor_ - 1];
}

std::optional<MapData> parseMap(std::istream& in) {
    MapData map;
    in >> map.id >> map.height >> map.width;
    if (!in || map.height <= 0 || map.width <= 0) return std::nullopt;

    const size_t cells = static_cast<size_t>(map.width) * map.height;
    std::string marker;
    auto readLayer = [&](std::vector<int>& layer) {
        layer.assign(cells, 0);
        for (int& value : layer) in >> value;
        in >> marker; // end-of-layer marker
    };
    readLayer(map.tiles);
    readLayer(map.properties);
    if (mapInfo(map.id).hasOverlay) readLayer(map.overlay);

    auto mark = [&map](int x, int y, TileProperty property) {
        if (x >= 0 && y >= 0 && x < map.width && y < map.height) map.properties[y * map.width + x] = property;
    };

    while (in >> marker && marker == "MAP_NEXT_NPC") {
        NpcData npc;
        std::string sprite;
        in >> npc.x >> npc.y >> npc.facing >> npc.isTrainer >> sprite;
        npc.sprite = assetPath(sprite);
        if (npc.isTrainer) {
            npc.preBattleDialogue = readBlock(in, "NPC_PREBATTLE_END");
            readLine(in, npc.trainerName);
            std::string trainerSprite;
            readLine(in, trainerSprite);
            npc.trainerSprite = assetPath(trainerSprite);
        }
        npc.dialogue = readBlock(in, "NPC_DIALOGUE_END");
        mark(npc.x, npc.y, OCCUPIED_BY_NPC);
        map.npcs.push_back(npc);
    }

    while (in >> marker && marker == "WARP_NEXT_TILE") {
        WarpTile warp{};
        in >> warp.x >> warp.y >> warp.destMap >> warp.destX >> warp.destY;
        mark(warp.x, warp.y, WARP);
        map.warps.push_back(warp);
    }

    while (in >> marker && marker == "INTER_NEXT_TILE") {
        int x, y;
        in >> x >> y;
        InterTile tile(x, y);
        tile.dialogue = readBlock(in, "INTER_DIALOGUE_END");
        mark(x, y, INTERACTIVE);
        map.interTiles.push_back(tile);
    }
    return map;
}
