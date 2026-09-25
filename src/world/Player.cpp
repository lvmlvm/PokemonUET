#include "world/Player.h"
#include "world/MapInfo.h"

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>

void HighScores::submit(int score) {
    bool beatsOne = std::any_of(scores_.begin(), scores_.end(), [score](int s) { return score > s; });
    if (!beatsOne) return;
    scores_.back() = score;
    std::sort(scores_.begin(), scores_.end(), std::greater<int>());
}

void Player::reset() {
    gender_ = 0;
    name_ = "Player";
    currentMap_ = E3_EXTERIOR;
    x_ = 20, y_ = 10;
    facing_ = 0;
    currentScore_ = 0;
    highScores.reset();
    for (Pokemon& pokemon : party) pokemon = Pokemon(0);
}

void Player::setGender(int gender) {
    if (gender == 0) name_ = "Ruby";
    else if (gender == 1) name_ = "Sapphire";
    else return;
    gender_ = gender;
}

void Player::healParty() {
    for (Pokemon& pokemon : party) pokemon = Pokemon(pokemon.species());
}

// Save format (text): name / gender / map / x y / party species x3 / current score / 5 high scores
bool Player::load(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        std::cout << "No save file detected! Default player config loaded instead!\n";
        return false;
    }

    Player loaded;
    int partyIds[3] = {0, 0, 0};
    std::getline(in, loaded.name_);
    in >> loaded.gender_ >> loaded.currentMap_ >> loaded.x_ >> loaded.y_;
    in >> partyIds[0] >> partyIds[1] >> partyIds[2];
    in >> loaded.currentScore_;
    for (int& score : loaded.highScores.scores()) in >> score;

    // Reject corrupted saves instead of indexing out of bounds with their values.
    bool valid = !in.fail() && (loaded.gender_ == 0 || loaded.gender_ == 1)
        && loaded.currentMap_ >= 0 && loaded.currentMap_ < MAP_COUNT
        && loaded.x_ >= 0 && loaded.y_ >= 0 && loaded.currentScore_ >= 0;
    for (int id : partyIds) valid = valid && id >= 0 && id < psize;
    for (int score : loaded.highScores.scores()) valid = valid && score >= 0;
    if (!valid) {
        std::cout << "Save file is corrupted! Default player config loaded instead!\n";
        return false;
    }

    for (int i = 0; i < 3; i++) loaded.party[i] = Pokemon(partyIds[i]);
    *this = loaded;
    return true;
}

bool Player::save(const std::string& path) const {
    std::ofstream out(path);
    if (!out) return false;
    out << name_ << '\n';
    out << gender_ << '\n' << currentMap_ << '\n' << x_ << " " << y_ << '\n';
    out << party[0].species() << " " << party[1].species() << " " << party[2].species() << '\n';
    out << currentScore_ << '\n';
    for (int score : highScores.scores()) out << score << " ";
    out << '\n';
    return static_cast<bool>(out);
}
