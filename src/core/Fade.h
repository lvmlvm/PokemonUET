#pragma once

#include "core/Sdl.h"

// The black screen transition. Its alpha carries over between scenes: a scene fades out,
// the next one fades back in from where it left off.
class Fade {
public:
    explicit Fade(TexturePtr overlay);

    // Advance one frame. Each returns true on the frame after the fade finished, i.e. once
    // the screen has been fully black (out) or fully clear (in) for a whole frame.
    bool out(int step);
    bool in(int step);

    void draw(SDL_Renderer* renderer) const;

    int alpha() const { return alpha_; }
    void setAlpha(int alpha) { alpha_ = alpha; }

private:
    TexturePtr overlay_;
    int alpha_ = 0;
};
