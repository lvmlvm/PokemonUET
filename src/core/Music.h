#pragma once

#include "core/Sdl.h"

#include <string>

// The background theme. Themes play their intro once; every later loop restarts at the
// theme's repeat point.
class Music {
public:
    void load(const std::string& path, double repeatPoint = 0.0);
    void play();                          // (re)starts the theme when it isn't playing
    void resetChord() { pastChord_ = false; } // next play() starts from the beginning
    void stop();

private:
    MusicPtr theme_;
    double repeatPoint_ = 0.0;
    bool pastChord_ = false;
};
