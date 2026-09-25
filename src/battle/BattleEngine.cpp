#include "battle/BattleEngine.h"

#include <utility>

using Kind = BattleEvent::Kind;

BattleEngine::BattleEngine(Pokemon (&playerParty)[3], std::string playerName, Trainer& opponent, Rng& rng)
    : party_(playerParty), playerActive_(&playerParty[0]), playerName_(std::move(playerName)), opponent_(opponent), rng_(rng) {}

float BattleEngine::effectiveness(const Move& move, const Pokemon& target) {
    return static_cast<float>(typeEffectiveness[move.type][target.data->type] * typeEffectiveness[move.type][target.data->stype]);
}

float BattleEngine::damage(const Pokemon& attacker, const Move& move, const Pokemon& target, int randomPercent) {
    float stab = (move.type == attacker.data->type || move.type == attacker.data->stype) ? 1.5f : 1.0f;
    float te = effectiveness(move, target);
    return (move.power * attacker.data->atk / target.data->def / 2 * stab * te) * randomPercent / 100;
}

// Returns true if the target fainted.
bool BattleEngine::attack(std::vector<BattleEvent>& events, Pokemon& attacker, int moveIndex, Pokemon& target, bool byOpponent) {
    const Move& move = *attacker.data->move[moveIndex];
    events.push_back({byOpponent ? Kind::OpponentUsedMove : Kind::PlayerUsedMove,
                      std::string(byOpponent ? "Opposing " : "") + attacker.data->name + " used " + move.name + "!"});

    float te = effectiveness(move, target);
    target.c_hp -= damage(attacker, move, target, rng_.between(86, 100));
    attacker.c_pp[moveIndex]--;
    if (target.c_hp < 0) target.c_hp = 0;

    if (te == 0) {
        events.push_back({Kind::NoEffect, std::string(byOpponent ? "It didn't affect " : "It didn't affect the opposing ") + target.data->name + "..."});
    } else if (te < 1) {
        events.push_back({Kind::NotEffective, "It's not very effective..."});
    } else if (te > 1) {
        events.push_back({Kind::SuperEffective, "It's super effective!"});
    }
    return target.c_hp == 0;
}

int BattleEngine::chooseOpponentMove() {
    if (rng_.between(0, 1) == 0) return rng_.between(0, 3);

    // The move with the highest power * STAB * effectiveness (the first one on ties).
    const Pokemon& self = opponentPokemon();
    const Pokemon& target = *playerActive_;
    auto score = [&](const Move& move) {
        float stab = (move.type == self.data->type || move.type == self.data->stype) ? 1.5f : 1.0f;
        return move.power * effectiveness(move, target) * stab;
    };
    int best = 0;
    for (int i = 1; i < 4; i++) {
        if (score(*self.data->move[i]) > score(*self.data->move[best])) best = i;
    }
    return best;
}

void BattleEngine::opponentFainted(std::vector<BattleEvent>& events) {
    const Pokemon& fainted = opponentPokemon();
    events.push_back({Kind::OpponentFainted, "The opposing " + fainted.data->name + " fainted!"});
    events.push_back({Kind::ScoreGained, playerName_ + " scored " + std::to_string(fainted.data->hp) + " points!", fainted.data->hp});

    opponentIndex_++;
    if (opponentIndex_ < 3) {
        events.push_back({Kind::OpponentSentOut, opponent_.name + " sent out " + opponentPokemon().data->name + "!"});
    } else {
        opponentIndex_ = 2; // stays on the last Pokemon
        events.push_back({Kind::OpponentDefeated, playerName_ + " defeated " + opponent_.name + "!"});
        events.push_back({Kind::BattleOver, " "});
    }
}

void BattleEngine::playerFainted(std::vector<BattleEvent>& events) {
    events.push_back({Kind::PlayerFainted, playerActive_->data->name + " fainted!"});
    playerFaints_++;
    if (playerFaints_ < 3) {
        events.push_back({Kind::PlayerMustSwitch, "Awaiting " + playerName_ + "'s next Pokemon..."});
    } else {
        events.push_back({Kind::PlayerDefeated, playerName_ + " lost to " + opponent_.name + "!"});
        events.push_back({Kind::BattleOver, " "});
    }
}

std::vector<BattleEvent> BattleEngine::fight(int move) {
    std::vector<BattleEvent> events;
    Pokemon& mine = *playerActive_;
    Pokemon& theirs = opponentPokemon();
    if (mine.data->speed >= theirs.data->speed) {
        if (attack(events, mine, move, theirs, false)) {
            opponentFainted(events);
        } else {
            int theirMove = chooseOpponentMove();
            if (attack(events, theirs, theirMove, mine, true)) playerFainted(events);
        }
    } else {
        int theirMove = chooseOpponentMove();
        if (attack(events, theirs, theirMove, mine, true)) {
            playerFainted(events);
        } else if (attack(events, mine, move, theirs, false)) {
            opponentFainted(events);
        }
    }
    return events;
}

std::optional<std::vector<BattleEvent>> BattleEngine::switchTo(int slot) {
    Pokemon& chosen = party_[slot];
    if (chosen.c_hp == 0 || playerActive_ == &chosen) return std::nullopt;

    std::vector<BattleEvent> events;
    if (playerMustSwitch()) {
        playerActive_ = &chosen;
        events.push_back({Kind::PlayerSentOut, playerName_ + " sent out " + chosen.data->name + "!"});
    } else {
        events.push_back({Kind::PlayerWithdrew, playerName_ + " withdrew " + playerActive_->data->name + "!"});
        playerActive_ = &chosen;
        events.push_back({Kind::PlayerSentOut, playerName_ + " sent out " + chosen.data->name + "!", chosen.c_hp});
        int theirMove = chooseOpponentMove();
        if (attack(events, opponentPokemon(), theirMove, chosen, true)) playerFainted(events);
    }
    return events;
}
