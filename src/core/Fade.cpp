#include "core/Fade.h"

#include <utility>

Fade::Fade(TexturePtr overlay) : overlay_(std::move(overlay)) {
    SDL_SetTextureBlendMode(overlay_.get(), SDL_BLENDMODE_BLEND);
}

bool Fade::out(int step) {
    if (alpha_ < 255) {
        alpha_ += step;
        return false;
    }
    return true;
}

bool Fade::in(int step) {
    if (alpha_ > 0) {
        alpha_ -= step;
        return false;
    }
    alpha_ = 0;
    return true;
}

void Fade::draw(SDL_Renderer* renderer) const {
    SDL_SetTextureAlphaMod(overlay_.get(), static_cast<Uint8>(alpha_));
    SDL_RenderCopy(renderer, overlay_.get(), nullptr, nullptr);
}
