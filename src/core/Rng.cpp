#include "core/Rng.h"

#include <cstdlib>

Rng& gameRng() {
    static Rng rng([] {
        if (const char* seed = std::getenv("POKEMON_SEED")) return static_cast<unsigned int>(std::strtoul(seed, nullptr, 10));
        return static_cast<unsigned int>(std::random_device{}());
    }());
    return rng;
}
