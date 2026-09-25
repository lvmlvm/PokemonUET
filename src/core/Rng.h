#pragma once

#include <random>

// A seedable random number generator. The game uses one shared instance; tests can make
// their own with a fixed seed.
class Rng {
public:
    explicit Rng(unsigned int seed) : generator_(seed) {}

    int between(int lo, int hi) { // uniform in [lo, hi]
        return std::uniform_int_distribution<int>(lo, hi)(generator_);
    }

private:
    std::mt19937 generator_;
};

// The game-wide generator, seeded once from POKEMON_SEED if set (reproducible runs, e.g. the
// smoke test), otherwise from std::random_device.
Rng& gameRng();
inline int randomInt(int lo, int hi) { return gameRng().between(lo, hi); }
