#include <camera.h>

// Collision lookup that treats anything outside the map (or a missing map) as a wall.
static bool isBlocked(int** colMap, int mapWidth, int mapHeight, int x, int y) {
    if (colMap == NULL or x < 0 or y < 0 or x >= mapWidth or y >= mapHeight) return true;
    return colMap[y][x] != 0;
}

gameCam::gameCam() {
    camera = {0, 0, 64*13, 64*11};
    isMoving = 0, finishMove = 0, moveUp = 0, moveLeft = 0, moveDown = 0, moveRight = 0;
    movementSpeed = 4;
}

gameCam::~gameCam() {
    camera = {0, 0, 0, 0};
}

void gameCam::setCameraPos(int _x, int _y) {
    camera.x = _x;
    camera.y = _y;
}

void gameCam::beginMovement(SDL_Event* e, int x, int y, int** colMap, int mapWidth, int mapHeight) {
    switch (e->key.keysym.sym) {
        case SDLK_w:
            if (!isBlocked(colMap, mapWidth, mapHeight, x, y-1)) {moveUp = true; break;}
            else break;               
        case SDLK_a:     
            if (!isBlocked(colMap, mapWidth, mapHeight, x-1, y)) {moveLeft = true; break;}
            else break;                
        case SDLK_s:           
            if (!isBlocked(colMap, mapWidth, mapHeight, x, y+1)) {moveDown = true; break;}
            else break;         
        case SDLK_d:
            if (!isBlocked(colMap, mapWidth, mapHeight, x+1, y)) {moveRight = true; break;}                  
            else break;
        default:
            break;
    }
}

void gameCam::stopMovement(SDL_Event *e) {
    switch (e->key.keysym.sym) {
        case SDLK_w:                        
            finishMove = true; break;
        case SDLK_a:                        
            finishMove = true; break;
        case SDLK_s:                        
            finishMove = true; break;
        case SDLK_d:                        
            finishMove = true; break;
        default:
            break;
    }
}

void gameCam::moveCamera(int x, int y, int** colMap, int mapWidth, int mapHeight) {
    if (moveRight == true) {
        if (isBlocked(colMap, mapWidth, mapHeight, x+1, y)) {
            finishMovement();
        }
        else {
            isMoving = true; 
            camera.x += movementSpeed;
        }
    } else if (moveLeft == true) {
        if (isBlocked(colMap, mapWidth, mapHeight, x-1, y)) {
            finishMovement();
        }
        else {
            isMoving = true; 
            camera.x -= movementSpeed;
        } 
    } else if (moveUp == true) {
        if (isBlocked(colMap, mapWidth, mapHeight, x, y-1)) {
            finishMovement();
        }
        else {
            isMoving = true; 
            camera.y -= movementSpeed;
        }
    } else if (moveDown == true) {
        if (isBlocked(colMap, mapWidth, mapHeight, x, y+1)) {
            finishMovement();
        }
        else {
            isMoving = true; 
            camera.y += movementSpeed;
        }
    }
}

void gameCam::finishMovement() {
    if (moveRight == true) {
        if (camera.x % 64 != 0) {
            camera.x += movementSpeed;
        } else {
            moveRight = false;
            isMoving = false;
            finishMove = false;
        }
    } else if (moveLeft == true) {
        if (camera.x % 64 != 0) {
            camera.x -= movementSpeed;
        } else {
            isMoving = false;
            moveLeft = false;
            finishMove = false;
        }
    } else if (moveUp == true) {
        if (camera.y % 64 != 0) {
            camera.y -= movementSpeed;
        } else {
            isMoving = false;
            moveUp = false;
            finishMove = false;
        }
    } else if (moveDown == true) {
        if (camera.y % 64 != 0) {
            camera.y += movementSpeed;
        } else {
            isMoving = false;
            moveDown = false;
            finishMove = false;
        }
    }    
}

void gameCam::finishIllegalPos(int mapWidth, int mapHeight) { // safety check for when player moves out of bound
    if (camera.x > mapWidth*64 - 832) {
        camera.x -= 64;
    } else if (camera.y > mapHeight*64 - 704) {
        camera.y -= 64;
    } else if (camera.x < 0) {
        camera.x += 64;
    } else if (camera.y < 0) {
        camera.y += 64;
    }
}

void gameCam::speedUp() {
    if (camera.x % 8 == 0 and camera.y % 8 == 0) movementSpeed = 8;
}

void gameCam::slowDown() {
    movementSpeed = 4;
}

bool gameCam::getMovementState() {
    return isMoving;
}

bool gameCam::getFinishingState() {
    return finishMove;
}

int gameCam::getCamX() {
    return camera.x;
}

int gameCam::getCamY() {
    return camera.y;
}