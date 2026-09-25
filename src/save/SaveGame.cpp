#include "save/SaveGame.h"
#include "world/MapInfo.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace {
const char* HEADER = "PokemonUET save";
} // namespace

std::string SaveGame::serialize(const Player& player) {
    std::ostringstream out;
    out << HEADER << ' ' << VERSION << '\n';
    out << player.name_ << '\n';
    out << player.gender_ << ' ' << player.currentMap_ << ' ' << player.x_ << ' ' << player.y_ << '\n';
    out << player.party[0].species() << ' ' << player.party[1].species() << ' ' << player.party[2].species() << '\n';
    out << player.currentScore_ << '\n';
    for (int score : player.highScores.scores()) out << score << ' ';
    out << '\n';
    return out.str();
}

std::optional<Player> SaveGame::parse(std::istream& in) {
    Player player;
    std::string line;
    if (!std::getline(in, line)) return std::nullopt;
    if (line.rfind(HEADER, 0) == 0) { // versioned file: the name is on the next line
        std::istringstream header(line.substr(std::string(HEADER).size()));
        int version = 0;
        if (!(header >> version) || version != VERSION) return std::nullopt;
        if (!std::getline(in, line)) return std::nullopt;
    }
    if (!line.empty() && line.back() == '\r') line.pop_back();
    player.name_ = line;

    int partyIds[3] = {0, 0, 0};
    in >> player.gender_ >> player.currentMap_ >> player.x_ >> player.y_;
    in >> partyIds[0] >> partyIds[1] >> partyIds[2];
    in >> player.currentScore_;
    for (int& score : player.highScores.scores()) in >> score;

    bool valid = !in.fail() && (player.gender_ == 0 || player.gender_ == 1)
        && player.currentMap_ >= 0 && player.currentMap_ < MAP_COUNT
        && player.x_ >= 0 && player.y_ >= 0 && player.currentScore_ >= 0;
    for (int id : partyIds) valid = valid && id >= 0 && id < psize;
    for (int score : player.highScores.scores()) valid = valid && score >= 0;
    if (!valid) return std::nullopt;

    for (int i = 0; i < 3; i++) player.party[i] = Pokemon(partyIds[i]);
    return player;
}

std::optional<Player> SaveGame::readFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) return std::nullopt;
    return parse(in);
}

bool SaveGame::writeFile(const std::string& path, const Player& player) {
    const std::string temporary = path + ".tmp";
    {
        std::ofstream out(temporary, std::ios::trunc);
        if (!out) return false;
        out << serialize(player);
        out.flush();
        if (!out) return false;
    }
    std::error_code error;
    std::filesystem::rename(temporary, path, error); // replaces `path` if it exists
    if (error) {
        std::remove(temporary.c_str());
        return false;
    }
    return true;
}
