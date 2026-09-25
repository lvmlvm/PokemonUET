#pragma once

#include "core/Sdl.h"

#include <string>

// A line (or wrapped block) of text. The texture is only re-rendered when the text or color
// changes, instead of every frame.
class TextLabel {
public:
    TextLabel() = default;
    TextLabel(SDL_Renderer* renderer, TTF_Font* font) : renderer_(renderer), font_(font) {}

    void setText(const std::string& text, SDL_Color color);
    void draw(int x, int y);

private:
    static const int WRAP_WIDTH = 753;

    SDL_Renderer* renderer_ = nullptr;
    TTF_Font* font_ = nullptr;
    TexturePtr texture_;
    SDL_Rect rect_ = {0, 0, 0, 0};
    std::string text_;
    SDL_Color color_ = {0, 0, 0, 0};
    bool rendered_ = false;
};
