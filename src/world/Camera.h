#pragma once

#include "core/Sdl.h"

class Map;

// The overworld view. The player is always drawn at the center of the screen (tile (6, 5) of
// the 13x11-tile view), so moving the player means scrolling the camera one 64px tile at a time.
class Camera {
public:
    static const int VIEW_TILES_X = 13, VIEW_TILES_Y = 11;
    static const int PLAYER_OFFSET_X = 6, PLAYER_OFFSET_Y = 5;

    void centerOn(int tileX, int tileY);
    void beginMovement(SDL_Keycode key, int playerX, int playerY, const Map& map);
    void stopMovement(SDL_Keycode key);
    void move(int playerX, int playerY, const Map& map);
    void finishMovement();                          // keep going until aligned to the tile grid
    void clampToMap(int mapWidth, int mapHeight);   // safety net when moving out of bounds
    void speedUp();
    void slowDown();

    bool isMoving() const { return isMoving_; }
    bool isFinishing() const { return finishMove_; }
    int x() const { return rect_.x; }
    int y() const { return rect_.y; }
    int playerTileX() const { return rect_.x / 64 + PLAYER_OFFSET_X; }
    int playerTileY() const { return rect_.y / 64 + PLAYER_OFFSET_Y; }

private:
    SDL_Rect rect_ = {0, 0, 64 * VIEW_TILES_X, 64 * VIEW_TILES_Y};
    bool isMoving_ = false, finishMove_ = false;
    bool moveUp_ = false, moveLeft_ = false, moveDown_ = false, moveRight_ = false;
    int speed_ = 4;
};
