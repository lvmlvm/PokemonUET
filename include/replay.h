#pragma once

// Developer tool: scripted input playback + screenshots, enabled with POKEMON_REPLAY=<script>.
//
// Script format, one command per line ('#' starts a comment), times in ms since startup:
//   <ms> down <key>        key press   (key names as in SDL_GetKeyFromName: W, X, V, Escape, ...)
//   <ms> up <key>          key release
//   <ms> tap <key>         press, released 80 ms later
//   <ms> click <x> <y>     mouse move + left click at window coordinates
//   <ms> shot <file.bmp>   save the next rendered frame
//   <ms> quit              push SDL_QUIT
namespace replay {
    void load();           // reads POKEMON_REPLAY if set; no-op otherwise
    void beforePresent();  // call once per frame, right before SDL_RenderPresent
}
