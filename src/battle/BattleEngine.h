#pragma once

#include "battle/Pokemon.h"
#include "core/Rng.h"

#include <optional>
#include <string>
#include <vector>

// Something that happened in a battle, in the order it happened. Each event is shown as one
// line of battle dialogue, with the matching animation.
struct BattleEvent {
    enum class Kind {
        PlayerUsedMove, OpponentUsedMove,
        NoEffect, NotEffective, SuperEffective,   // follows the move it describes; normal hits have none
        OpponentFainted, ScoreGained, OpponentSentOut, OpponentDefeated,
        PlayerFainted, PlayerMustSwitch, PlayerDefeated,
        PlayerWithdrew, PlayerSentOut,
        BattleOver,
    };

    Kind kind;
    std::string text;
    int value = 0; // ScoreGained: points. PlayerSentOut after a withdrawal: the new Pokemon's HP before the opponent's free hit.
};

// The rules of a 3-vs-3 trainer battle, independent of how it is shown.
//
// Each turn the faster Pokemon attacks first (the player wins speed ties) and a Pokemon that
// faints doesn't attack. Damage = power * atk / def / 2, times 1.5 for a move of the user's
// own type (STAB), times the type effectiveness against both of the target's types, times a
// random 86-100%. The opponent picks a random move half the time and its strongest move
// otherwise. Switching Pokemon voluntarily gives the opponent a free attack.
class BattleEngine {
public:
    BattleEngine(Pokemon (&playerParty)[3], std::string playerName, Trainer& opponent, Rng& rng);

    std::vector<BattleEvent> fight(int move);
    // Sends out party member `slot`: after a faint (free), or instead of attacking. Returns
    // nothing if that Pokemon can't battle or is already out.
    std::optional<std::vector<BattleEvent>> switchTo(int slot);

    Pokemon& playerPokemon() { return *playerActive_; }
    const Pokemon& playerPokemon() const { return *playerActive_; }
    Pokemon& opponentPokemon() { return opponent_.party[opponentIndex_]; }
    const Pokemon& opponentPokemon() const { return opponent_.party[opponentIndex_]; }
    int opponentIndex() const { return opponentIndex_; }
    bool playerMustSwitch() const { return playerActive_->c_hp == 0; }

    // Exposed for tests. The damage is fractional; it is subtracted from the target's HP and
    // the result rounded toward zero.
    static float damage(const Pokemon& attacker, const Move& move, const Pokemon& target, int randomPercent);
    static float effectiveness(const Move& move, const Pokemon& target);

private:
    bool attack(std::vector<BattleEvent>& events, Pokemon& attacker, int move, Pokemon& target, bool byOpponent);
    int chooseOpponentMove();
    void opponentFainted(std::vector<BattleEvent>& events);
    void playerFainted(std::vector<BattleEvent>& events);

    Pokemon* party_;
    Pokemon* playerActive_;
    std::string playerName_;
    Trainer& opponent_;
    Rng& rng_;
    int opponentIndex_ = 0;
    int playerFaints_ = 0;
};
