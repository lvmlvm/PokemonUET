#pragma once

// Single game-wide random number generator. Seeded once, from POKEMON_SEED if set
// (reproducible runs, e.g. the smoke test), otherwise from std::random_device.
int randomInt(int lo, int hi); // uniform in [lo, hi]
