#pragma once

#include "core/Sdl.h"

// Initializes SDL and its image/ttf/mixer extensions, and owns the game window and renderer.
// Everything that holds SDL resources must be destroyed before this object.
class SdlContext {
public:
    SdlContext(const char* title, int width, int height); // throws std::runtime_error
    SdlContext(const SdlContext&) = delete;
    SdlContext& operator=(const SdlContext&) = delete;

    SDL_Renderer* renderer() const { return renderer_.get(); }

private:
    struct Libraries {
        Libraries();
        ~Libraries();
    };

    Libraries libraries_; // declared first: initialized before, and shut down after, the window
    WindowPtr window_;
    RendererPtr renderer_;
};
