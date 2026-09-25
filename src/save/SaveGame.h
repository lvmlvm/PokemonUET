#pragma once

#include "world/Player.h"

#include <iosfwd>
#include <optional>
#include <string>

// Reading and writing the player's progress. No SDL dependency.
//
// Format (text), version 2:
//   PokemonUET save 2
//   <name>
//   <gender> <map> <x> <y>
//   <party species> x3
//   <current score>
//   <high scores> x5
// Version 1 files (the original game's) are the same without the header line, with map and
// position on separate lines; they are still read.
class SaveGame {
public:
    static const int VERSION = 2;

    static std::string serialize(const Player& player);
    // Nothing if the data is malformed or out of range.
    static std::optional<Player> parse(std::istream& in);

    static std::optional<Player> readFile(const std::string& path);
    // Writes to a temporary file first and then replaces `path`, so an interrupted save never
    // leaves a half-written file behind.
    static bool writeFile(const std::string& path, const Player& player);
};
