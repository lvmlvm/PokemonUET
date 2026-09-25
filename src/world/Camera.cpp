#include "world/Camera.h"
#include "world/Map.h"

namespace {
bool isBlocked(const Map& map, int x, int y) {
    return map.property(x, y) != WALKABLE;
}
} // namespace

void Camera::centerOn(int tileX, int tileY) {
    rect_.x = (tileX - PLAYER_OFFSET_X) * 64;
    rect_.y = (tileY - PLAYER_OFFSET_Y) * 64;
}

void Camera::beginMovement(SDL_Keycode key, int x, int y, const Map& map) {
    switch (key) {
        case SDLK_w: if (!isBlocked(map, x, y - 1)) moveUp_ = true; break;
        case SDLK_a: if (!isBlocked(map, x - 1, y)) moveLeft_ = true; break;
        case SDLK_s: if (!isBlocked(map, x, y + 1)) moveDown_ = true; break;
        case SDLK_d: if (!isBlocked(map, x + 1, y)) moveRight_ = true; break;
        default: break;
    }
}

void Camera::stopMovement(SDL_Keycode key) {
    if (key == SDLK_w || key == SDLK_a || key == SDLK_s || key == SDLK_d) finishMove_ = true;
}

void Camera::move(int x, int y, const Map& map) {
    if (moveRight_) {
        if (isBlocked(map, x + 1, y)) finishMovement();
        else { isMoving_ = true; rect_.x += speed_; }
    } else if (moveLeft_) {
        if (isBlocked(map, x - 1, y)) finishMovement();
        else { isMoving_ = true; rect_.x -= speed_; }
    } else if (moveUp_) {
        if (isBlocked(map, x, y - 1)) finishMovement();
        else { isMoving_ = true; rect_.y -= speed_; }
    } else if (moveDown_) {
        if (isBlocked(map, x, y + 1)) finishMovement();
        else { isMoving_ = true; rect_.y += speed_; }
    }
}

void Camera::finishMovement() {
    auto settle = [this](bool& direction, int& coordinate, int step) {
        if (coordinate % 64 != 0) {
            coordinate += step;
        } else {
            direction = false;
            isMoving_ = false;
            finishMove_ = false;
        }
    };
    if (moveRight_) settle(moveRight_, rect_.x, speed_);
    else if (moveLeft_) settle(moveLeft_, rect_.x, -speed_);
    else if (moveUp_) settle(moveUp_, rect_.y, -speed_);
    else if (moveDown_) settle(moveDown_, rect_.y, speed_);
}

void Camera::clampToMap(int mapWidth, int mapHeight) {
    if (rect_.x > mapWidth * 64 - 64 * VIEW_TILES_X) rect_.x -= 64;
    else if (rect_.y > mapHeight * 64 - 64 * VIEW_TILES_Y) rect_.y -= 64;
    else if (rect_.x < 0) rect_.x += 64;
    else if (rect_.y < 0) rect_.y += 64;
}

void Camera::speedUp() {
    if (rect_.x % 8 == 0 && rect_.y % 8 == 0) speed_ = 8;
}

void Camera::slowDown() {
    speed_ = 4;
}
