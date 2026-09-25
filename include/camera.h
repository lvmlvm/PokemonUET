#pragma once

#include <SDL.h>

class gameCam {
    private:
        SDL_Rect camera;
        bool isMoving, finishMove;
        bool moveUp, moveLeft, moveDown, moveRight;
        int movementSpeed;
    public:
        gameCam();
        ~gameCam();

        void setCameraPos(int _x = 0, int _y = 0);
        void beginMovement(SDL_Event* e, int x, int y, int** colMap);
        void stopMovement(SDL_Event* e);
        void moveCamera(int x, int y, int** colMap);
        void finishMovement();
        void finishIllegalPos(int mapWidth, int mapHeight);
        void speedUp();
        void slowDown();

        bool getMovementState();
        bool getFinishingState();
        int getCamX();
        int getCamY();
};