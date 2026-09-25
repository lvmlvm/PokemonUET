#pragma once

#include "core/Sdl.h"

#include <string>
#include <vector>

// A character standing on the map. Trainers battle the player after their pre-battle lines.
class Npc {
public:
    enum class Talk { Talking, Finished, StartBattle };

    Npc(int x, int y, int facing, SDL_Texture* spriteSheet, bool isTrainer = false);

    Talk talk(int playerFacing); // advances the dialogue by one sentence
    std::string currentSentence() const;
    void draw(SDL_Renderer* renderer, int camX, int camY) const;

    void addDialogue(const std::string& sentence) { dialogue_.push_back(sentence); }
    void addPreBattleDialogue(const std::string& sentence) { preBattleDialogue_.push_back(sentence); }

    int x() const { return x_; }
    int y() const { return y_; }
    bool isTrainer() const { return isTrainer_; }
    bool hasBattled() const { return hasBattled_; }
    void markBattled() { hasBattled_ = true; }

    std::string trainerName;
    std::string trainerSprite; // asset path of the battle sprite

private:
    const std::vector<std::string>& activeDialogue() const;

    int x_, y_;
    int facing_;
    SDL_Texture* spriteSheet_;
    bool isTrainer_;
    bool hasBattled_ = false;
    std::vector<std::string> dialogue_;
    std::vector<std::string> preBattleDialogue_;
    unsigned int cursor_ = 0; // 1-based index of the sentence being shown, 0 = not talking
};

struct WarpTile {
    int x, y;
    int destMap, destX, destY;
};

// A tile the player can inspect (signs, machines, ...).
class InterTile {
public:
    InterTile(int x, int y) : x_(x), y_(y) {}

    bool talk(); // advances the dialogue; false once it is over
    std::string currentSentence() const;
    unsigned int sentenceNumber() const { return cursor_; }

    int x() const { return x_; }
    int y() const { return y_; }

    std::vector<std::string> dialogue;

private:
    int x_, y_;
    unsigned int cursor_ = 0;
};
