#include "battle/BattleEngine.h"

#include <doctest/doctest.h>

#include <vector>

using Kind = BattleEvent::Kind;

namespace {
// Species used below (indices into pokemonData).
const int TAUROS = 1;    // Normal, speed 115; moves Body Slam, Sacred Sword, Earthquake, Darkest Lariat
const int SNORLAX = 2;   // Normal, 220 HP, speed 35
const int CHARIZARD = 3; // Fire/Flying
const int VENUSAUR = 16; // Poison/Grass

Trainer trainerWith(int species, int hp = -1) {
    Trainer trainer;
    trainer.name = "Rival";
    for (Pokemon& pokemon : trainer.party) {
        pokemon = Pokemon(species);
        if (hp >= 0) pokemon.c_hp = hp;
    }
    return trainer;
}

void fillParty(Pokemon (&party)[3], int species, int hp = -1) {
    for (Pokemon& pokemon : party) {
        pokemon = Pokemon(species);
        if (hp >= 0) pokemon.c_hp = hp;
    }
}

// The event kinds, without the effectiveness remarks (which depend on the move rolled).
std::vector<Kind> kinds(const std::vector<BattleEvent>& events) {
    std::vector<Kind> result;
    for (const BattleEvent& event : events) {
        if (event.kind != Kind::NoEffect && event.kind != Kind::NotEffective && event.kind != Kind::SuperEffective) {
            result.push_back(event.kind);
        }
    }
    return result;
}

const Move& moveOf(int species, int slot) {
    return *pokemonData[species].move[slot];
}
} // namespace

TEST_CASE("type chart") {
    Pokemon charizard(CHARIZARD), venusaur(VENUSAUR), tauros(TAUROS);
    const Move& flamethrower = moveOf(CHARIZARD, 0);
    const Move& earthquake = moveOf(CHARIZARD, 1);
    const Move& bodySlam = moveOf(TAUROS, 0);

    CHECK(BattleEngine::effectiveness(flamethrower, venusaur) == 2.0f); // Fire vs Poison (1) x Grass (2)
    CHECK(BattleEngine::effectiveness(earthquake, charizard) == 0.0f);  // Ground vs Flying
    CHECK(BattleEngine::effectiveness(bodySlam, tauros) == 1.0f);
    CHECK(typeEffectiveness[1][14] == 0.0);  // Normal vs Ghost
    CHECK(typeEffectiveness[3][2] == 2.0);   // Water vs Fire
    CHECK(typeEffectiveness[4][9] == 0.0);   // Electric vs Ground
}

TEST_CASE("damage: base power, attack/defense, STAB, effectiveness and the random factor") {
    Pokemon charizard(CHARIZARD), venusaur(VENUSAUR);
    const Move& flamethrower = moveOf(CHARIZARD, 0);
    // 90 * 114 / 105 / 2 = 48 (integer division), x1.5 STAB, x2 effectiveness
    CHECK(BattleEngine::damage(charizard, flamethrower, venusaur, 100) == doctest::Approx(144.0));
    CHECK(BattleEngine::damage(charizard, flamethrower, venusaur, 86) == doctest::Approx(123.84));

    const Move& earthquake = moveOf(CHARIZARD, 1);
    CHECK(BattleEngine::damage(venusaur, earthquake, charizard, 100) == 0.0f); // immune
}

TEST_CASE("the faster Pokemon attacks first; the player wins speed ties") {
    Rng rng(1);
    Pokemon party[3];

    fillParty(party, TAUROS);
    Trainer slow = trainerWith(SNORLAX);
    CHECK(BattleEngine(party, "Ruby", slow, rng).fight(0).front().kind == Kind::PlayerUsedMove);

    fillParty(party, SNORLAX);
    Trainer fast = trainerWith(TAUROS);
    CHECK(BattleEngine(party, "Ruby", fast, rng).fight(0).front().kind == Kind::OpponentUsedMove);

    fillParty(party, TAUROS);
    Trainer same = trainerWith(TAUROS);
    CHECK(BattleEngine(party, "Ruby", same, rng).fight(0).front().kind == Kind::PlayerUsedMove);
}

TEST_CASE("an attack hits for 86-100% of the damage and uses up PP") {
    Rng rng(7);
    Pokemon party[3];
    fillParty(party, CHARIZARD);
    Trainer opponent = trainerWith(VENUSAUR, 1000);
    BattleEngine engine(party, "Ruby", opponent, rng);

    std::vector<BattleEvent> events = engine.fight(0); // Charizard is faster
    CHECK(events[0].text == "Charizard used Flamethrower!");
    CHECK(events[1].kind == Kind::SuperEffective);
    CHECK(opponent.party[0].c_hp >= 1000 - 144);
    CHECK(opponent.party[0].c_hp <= 1000 - 123);
    CHECK(party[0].c_pp[0] == moveOf(CHARIZARD, 0).pp - 1);
}

TEST_CASE("knocking out the opponent's Pokemon scores its HP, then the next one comes out") {
    Rng rng(3);
    Pokemon party[3];
    fillParty(party, TAUROS);
    Trainer opponent = trainerWith(SNORLAX, 1);
    BattleEngine engine(party, "Ruby", opponent, rng);

    std::vector<BattleEvent> first = engine.fight(0);
    CHECK(kinds(first) == std::vector<Kind>{Kind::PlayerUsedMove, Kind::OpponentFainted, Kind::ScoreGained, Kind::OpponentSentOut});
    CHECK(first[2].value == 220);
    CHECK(first[2].text == "Ruby scored 220 points!");
    CHECK(engine.opponentIndex() == 1);

    engine.fight(0);
    std::vector<BattleEvent> last = engine.fight(0);
    CHECK(kinds(last) == std::vector<Kind>{Kind::PlayerUsedMove, Kind::OpponentFainted, Kind::ScoreGained,
                                           Kind::OpponentDefeated, Kind::BattleOver});
    CHECK(last[3].text == "Ruby defeated Rival!");
    CHECK(engine.opponentIndex() == 2);
}

TEST_CASE("a fainted Pokemon must be replaced; losing all three ends the battle") {
    Rng rng(5);
    Pokemon party[3];
    fillParty(party, SNORLAX, 1);
    Trainer opponent = trainerWith(TAUROS); // faster, and all its moves hit Normal types
    BattleEngine engine(party, "Ruby", opponent, rng);

    CHECK(kinds(engine.fight(0)) == std::vector<Kind>{Kind::OpponentUsedMove, Kind::PlayerFainted, Kind::PlayerMustSwitch});
    CHECK(engine.playerMustSwitch());
    CHECK_FALSE(engine.switchTo(0).has_value()); // fainted

    std::optional<std::vector<BattleEvent>> sentOut = engine.switchTo(1);
    REQUIRE(sentOut.has_value());
    CHECK(kinds(*sentOut) == std::vector<Kind>{Kind::PlayerSentOut}); // no free hit after a faint
    CHECK(&engine.playerPokemon() == &party[1]);

    engine.fight(0);
    REQUIRE(engine.switchTo(2).has_value());
    CHECK(kinds(engine.fight(0)) == std::vector<Kind>{Kind::OpponentUsedMove, Kind::PlayerFainted, Kind::PlayerDefeated, Kind::BattleOver});
}

TEST_CASE("switching voluntarily gives the opponent a free attack") {
    Rng rng(11);
    Pokemon party[3];
    fillParty(party, SNORLAX);
    Trainer opponent = trainerWith(SNORLAX);
    BattleEngine engine(party, "Ruby", opponent, rng);

    CHECK_FALSE(engine.switchTo(0).has_value()); // already out
    std::optional<std::vector<BattleEvent>> events = engine.switchTo(1);
    REQUIRE(events.has_value());
    CHECK(kinds(*events) == std::vector<Kind>{Kind::PlayerWithdrew, Kind::PlayerSentOut, Kind::OpponentUsedMove});
    CHECK((*events)[1].value == 220); // HP before the free hit
    CHECK(party[1].c_hp < 220);
    CHECK(party[0].c_hp == 220);
}

TEST_CASE("the same seed plays out the same battle") {
    auto play = [](unsigned int seed) {
        Rng rng(seed);
        Pokemon party[3];
        fillParty(party, CHARIZARD);
        Trainer opponent = trainerWith(VENUSAUR);
        BattleEngine engine(party, "Ruby", opponent, rng);
        std::vector<std::string> lines;
        for (int turn = 0; turn < 3; turn++) {
            for (const BattleEvent& event : engine.fight(turn % 4)) lines.push_back(event.text);
        }
        lines.push_back(std::to_string(party[0].c_hp) + " " + std::to_string(engine.opponentPokemon().c_hp));
        return lines;
    };
    CHECK(play(42) == play(42));
}
