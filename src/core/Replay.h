#pragma once

// Developer tool: scripted input playback + screenshots, enabled with POKEMON_REPLAY=<script>.
//
// Script format, one command per line ('#' starts a comment). Times are in rendered frames
// since startup (the game logic is frame-based, so scripts stay in sync even on slow builds):
//   <frame> down <key>     key press   (key names as in SDL_GetKeyFromName: W, X, V, Escape, ...)
//   <frame> up <key>       key release
//   <frame> tap <key>      press, released 5 frames later
//   <frame> click <x> <y>  mouse move + left click at window coordinates
//   <frame> shot <file.bmp> save the next rendered frame
//   <frame> quit           push SDL_QUIT
//
// POKEMON_REPLAY_SPEED=<factor> scales the game's sleeps (2 = double speed, 0 = no sleeping).
// Game logic is frame-based, so a replay behaves identically at any speed.
//
// Watchdog: if the game is still running WATCHDOG_FRAMES after the script's last command,
// the harness pushes SDL_QUIT; if that doesn't end the game within another WATCHDOG_FRAMES,
// it aborts the process (exit code 3), so a stuck state never hangs an unattended run.
#include <SDL.h>

namespace replay {
    void load();           // reads POKEMON_REPLAY if set; no-op otherwise
    void beforePresent(SDL_Renderer* renderer); // call once per frame, right before SDL_RenderPresent
    void delay(Uint32 ms); // SDL_Delay, skipped in turbo mode
}
