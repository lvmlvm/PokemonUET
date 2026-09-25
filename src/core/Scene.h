#pragma once

#include "core/Sdl.h"

// One screen of the game (title, overworld, battle, ...). The Game runs the scene on top of
// its stack: events first, then one frame of update + rendering.
class Scene {
public:
    virtual ~Scene() = default;

    virtual void handleEvent(const SDL_Event& event) = 0;
    virtual void frame() = 0;
    virtual void onQuit() {}   // the window is being closed
    virtual void onResume() {} // the scene above this one was popped
};
