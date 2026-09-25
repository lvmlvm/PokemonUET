#include "save/SaveGame.h"
#include "world/MapInfo.h"

#include <doctest/doctest.h>

#include <filesystem>
#include <sstream>

namespace {
std::optional<Player> parseText(const std::string& text) {
    std::istringstream in(text);
    return SaveGame::parse(in);
}
} // namespace

TEST_CASE("high scores keep the five best, highest first") {
    HighScores scores;
    scores.submit(320);
    scores.submit(100);
    scores.submit(500);
    CHECK(scores.scores() == std::array<int, 5>{500, 320, 100, 0, 0});

    for (int score : {50, 60, 70}) scores.submit(score);
    CHECK(scores.scores() == std::array<int, 5>{500, 320, 100, 70, 60});
    scores.submit(10); // lower than all of them
    CHECK(scores.scores() == std::array<int, 5>{500, 320, 100, 70, 60});
}

TEST_CASE("a save round-trips") {
    Player player;
    player.setGender(1);
    player.setPosition(12, 22, 5);
    player.party[0] = Pokemon(3);
    player.party[1] = Pokemon(16);
    player.party[2] = Pokemon(36);
    player.addScore(155);
    player.highScores.submit(900);

    std::optional<Player> loaded = parseText(SaveGame::serialize(player));
    REQUIRE(loaded.has_value());
    CHECK(loaded->name() == "Sapphire");
    CHECK(loaded->gender() == 1);
    CHECK(loaded->currentMap() == 5);
    CHECK(loaded->x() == 12);
    CHECK(loaded->y() == 22);
    CHECK(loaded->party[0].species() == 3);
    CHECK(loaded->party[1].species() == 16);
    CHECK(loaded->party[2].species() == 36);
    CHECK(loaded->currentScore() == 155);
    CHECK(loaded->highScores.scores() == std::array<int, 5>{900, 0, 0, 0, 0});
}

TEST_CASE("saves from the original game (version 1, no header) still load") {
    std::optional<Player> loaded = parseText("Ruby\n0\n1\n20 10\n12 8 14\n0\n320 0 0 0 0 \n");
    REQUIRE(loaded.has_value());
    CHECK(loaded->name() == "Ruby");
    CHECK(loaded->currentMap() == E3_EXTERIOR);
    CHECK(loaded->x() == 20);
    CHECK(loaded->y() == 10);
    CHECK(loaded->party[0].species() == 12);
    CHECK(loaded->highScores.scores()[0] == 320);
}

TEST_CASE("corrupted saves are rejected") {
    const std::string valid = "PokemonUET save 2\nRuby\n0 1 20 10\n12 8 14\n0\n320 0 0 0 0\n";
    CHECK(parseText(valid).has_value());

    CHECK_FALSE(parseText("PokemonUET save 3\nRuby\n0 1 20 10\n12 8 14\n0\n320 0 0 0 0\n").has_value()); // unknown version
    CHECK_FALSE(parseText("PokemonUET save 2\nRuby\n0 1 20 10\n12 99 14\n0\n320 0 0 0 0\n").has_value()); // no such Pokemon
    CHECK_FALSE(parseText("PokemonUET save 2\nRuby\n0 12 20 10\n12 8 14\n0\n320 0 0 0 0\n").has_value()); // no such map
    CHECK_FALSE(parseText("PokemonUET save 2\nRuby\n2 1 20 10\n12 8 14\n0\n320 0 0 0 0\n").has_value());  // bad gender
    CHECK_FALSE(parseText("PokemonUET save 2\nRuby\n0 1 -3 10\n12 8 14\n0\n320 0 0 0 0\n").has_value());  // negative position
    CHECK_FALSE(parseText("PokemonUET save 2\nRuby\n0 1 20 10\n12 8").has_value());                       // truncated
    CHECK_FALSE(parseText("").has_value());
}

TEST_CASE("saving writes the file atomically and leaves no temporary file") {
    namespace fs = std::filesystem;
    const fs::path dir = fs::temp_directory_path() / "pokemon_uet_save_test";
    fs::remove_all(dir);
    fs::create_directories(dir);
    const std::string path = (dir / "player.sav").string();

    Player player;
    player.setGender(0);
    REQUIRE(SaveGame::writeFile(path, player));
    player.addScore(42);
    REQUIRE(SaveGame::writeFile(path, player)); // replaces the existing file

    std::optional<Player> loaded = SaveGame::readFile(path);
    REQUIRE(loaded.has_value());
    CHECK(loaded->currentScore() == 42);
    CHECK_FALSE(fs::exists(path + ".tmp"));
    CHECK_FALSE(SaveGame::readFile((dir / "missing.sav").string()).has_value());
    fs::remove_all(dir);
}
