#include "world/Player.h"
#include "world/MapInfo.h"

#include <algorithm>
#include <functional>

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
