#pragma once

#include "battle/Pokemon.h"

#include <array>
#include <string>

// The five best scores, highest first.
class HighScores {
public:
    // Takes the place of the lowest score if it beats any of them.
    void submit(int score);
    void reset() { scores_.fill(0); }

    const std::array<int, 5>& scores() const { return scores_; }
    std::array<int, 5>& scores() { return scores_; }

private:
    std::array<int, 5> scores_ = {};
};

class Player {
public:
    Player() { reset(); }

    void reset();                    // a brand-new game

    void setGender(int gender);      // 0 = Ruby, 1 = Sapphire; also sets the name
    int gender() const { return gender_; }
    const std::string& name() const { return name_; }

    int currentMap() const { return currentMap_; }
    int x() const { return x_; }
    int y() const { return y_; }
    void setPosition(int x, int y, int mapId) { x_ = x; y_ = y; currentMap_ = mapId; }

    int facing() const { return facing_; }
    void setFacing(int facing) { facing_ = facing; }

    bool hasPokemon() const { return party[0].species() != 0; }
    void healParty(); // back to full HP and PP

    int currentScore() const { return currentScore_; }
    void addScore(int points) { currentScore_ += points; }
    // The run is over: record its score and start from zero.
    void endRun() { highScores.submit(currentScore_); currentScore_ = 0; }

    Pokemon party[3];
    HighScores highScores;

private:
    friend class SaveGame;

    int gender_ = 0;
    std::string name_;
    int currentMap_ = 0;
    int x_ = 0, y_ = 0;
    int facing_ = 0;
    int currentScore_ = 0;
};
