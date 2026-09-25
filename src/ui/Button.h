#pragma once

#include "core/Sdl.h"

#include <array>

// A clickable image. Sprite-sheet buttons have three frames (normal, hovered, pressed);
// single-image buttons just draw the whole texture. The click flag stays set until the
// owner resets it.
class Button {
public:
    Button() = default;

    // The whole texture at (x, y), sized like the texture.
    static Button single(SDL_Texture* texture, int x, int y);
    // Frames stacked top to bottom / left to right in the texture, drawn into `dest`.
    static Button vertical(SDL_Texture* texture, SDL_Rect dest, int frameWidth, int frameHeight);
    static Button horizontal(SDL_Texture* texture, SDL_Rect dest, int frameWidth, int frameHeight);

    void handleEvent(const SDL_Event& event);
    void draw(SDL_Renderer* renderer) const;

    bool clicked() const { return clicked_; }
    void resetClick() { clicked_ = false; }

    void resetFrame() { frame_ = 0; }
    void setPosition(int x, int y) { dest_.x = x; dest_.y = y; }
    const SDL_Rect& rect() const { return dest_; }
    bool contains(int x, int y) const;
    SDL_Texture* texture() const { return texture_; }

private:
    SDL_Texture* texture_ = nullptr;
    SDL_Rect dest_ = {0, 0, 0, 0};
    std::array<SDL_Rect, 3> frames_ = {};
    bool wholeTexture_ = true;
    int frame_ = 0;
    bool clicked_ = false;
};

// Mouse position carried by a mouse motion/button event (works for real and replayed events).
void eventMousePosition(const SDL_Event& event, int& x, int& y);
