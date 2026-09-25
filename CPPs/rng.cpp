#include "rng.h"

#include <cstdlib>
#include <random>

static std::mt19937& generator() {
    static std::mt19937 gen = [] {
        if (const char* seed = std::getenv("POKEMON_SEED")) return std::mt19937(std::strtoul(seed, nullptr, 10));
        return std::mt19937(std::random_device{}());
    }();
    return gen;
}

int randomInt(int lo, int hi) {
    return std::uniform_int_distribution<int>(lo, hi)(generator());
}
