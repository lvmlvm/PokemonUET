#include "ui/TextLabel.h"

void TextLabel::setText(const std::string& text, SDL_Color color) {
    if (rendered_ && text == text_ && color.r == color_.r && color.g == color_.g && color.b == color_.b && color.a == color_.a) return;
    text_ = text;
    color_ = color;
    rendered_ = true;
    texture_.reset();
    if (font_ == nullptr) return;

    SurfacePtr surface(TTF_RenderText_Blended_Wrapped(font_, text.c_str(), color, WRAP_WIDTH));
    if (!surface) return; // e.g. empty text
    texture_.reset(SDL_CreateTextureFromSurface(renderer_, surface.get()));
    SDL_QueryTexture(texture_.get(), nullptr, nullptr, &rect_.w, &rect_.h);
}

void TextLabel::draw(int x, int y) {
    if (!texture_) return;
    rect_.x = x;
    rect_.y = y;
    SDL_RenderCopy(renderer_, texture_.get(), nullptr, &rect_);
}
