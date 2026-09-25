#include "ui/Button.h"

void eventMousePosition(const SDL_Event& event, int& x, int& y) {
    if (event.type == SDL_MOUSEMOTION) {
        x = event.motion.x;
        y = event.motion.y;
    } else {
        x = event.button.x;
        y = event.button.y;
    }
}

Button Button::single(SDL_Texture* texture, int x, int y) {
    Button button;
    button.texture_ = texture;
    button.dest_ = {x, y, 0, 0};
    SDL_QueryTexture(texture, nullptr, nullptr, &button.dest_.w, &button.dest_.h);
    return button;
}

Button Button::vertical(SDL_Texture* texture, SDL_Rect dest, int frameWidth, int frameHeight) {
    Button button;
    button.texture_ = texture;
    button.dest_ = dest;
    button.wholeTexture_ = false;
    for (int i = 0; i < 3; i++) button.frames_[i] = {0, frameHeight * i, frameWidth, frameHeight};
    return button;
}

Button Button::horizontal(SDL_Texture* texture, SDL_Rect dest, int frameWidth, int frameHeight) {
    Button button = vertical(texture, dest, frameWidth, frameHeight);
    for (int i = 0; i < 3; i++) button.frames_[i] = {frameWidth * i, 0, frameWidth, frameHeight};
    return button;
}

bool Button::contains(int x, int y) const {
    return x >= dest_.x && x <= dest_.x + dest_.w && y >= dest_.y && y <= dest_.y + dest_.h;
}

void Button::handleEvent(const SDL_Event& event) {
    if (event.type != SDL_MOUSEMOTION && event.type != SDL_MOUSEBUTTONDOWN && event.type != SDL_MOUSEBUTTONUP) return;
    int x = 0, y = 0;
    eventMousePosition(event, x, y);

    if (!contains(x, y)) {
        frame_ = 0;
    } else if (event.type == SDL_MOUSEMOTION) {
        frame_ = 1;
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        frame_ = 2;
    } else {
        frame_ = 1;
        clicked_ = true;
    }
}

void Button::draw(SDL_Renderer* renderer) const {
    if (texture_ == nullptr) return;
    SDL_RenderCopy(renderer, texture_, wholeTexture_ ? nullptr : &frames_[frame_], &dest_);
}
