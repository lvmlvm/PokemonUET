#include "core/RenderWindow.h"
#include "core/Replay.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
using namespace std;

SDL_Renderer* RenderWindow::renderer = nullptr;

RenderWindow::RenderWindow() {
    renderer = NULL;
    window = NULL;
}

RenderWindow::~RenderWindow() {
    // Nothing to do: main() calls close() explicitly, before global objects are destroyed.
}

void RenderWindow::create(const char *title, int w, int h) {
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        return;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) { // e.g. headless/offscreen video drivers
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (renderer == NULL) SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
}

SDL_Texture* RenderWindow::loadTexture(const char* path) {
    SDL_Texture* texture = NULL;
    texture = IMG_LoadTexture(renderer, path);

    return texture;
}

void RenderWindow::clear() {
    SDL_RenderClear(renderer);
}

void RenderWindow::drawColor(int red, int green, int blue) {
    SDL_SetRenderDrawColor(renderer, red, green, blue, 255);
}

void RenderWindow::render(SDL_Texture* texture, SDL_Rect* clip) {
    SDL_RenderCopy(renderer, texture, NULL, clip);
}

void RenderWindow::display() {
    replay::beforePresent();
    SDL_RenderPresent(renderer);
}

void RenderWindow::close() {
    if (window == NULL) return; // already closed
    if (renderer != NULL) SDL_DestroyRenderer(renderer);
    renderer = NULL;
    SDL_DestroyWindow(window);
    window = NULL;

    Mix_CloseAudio();
    TTF_Quit();
    Mix_Quit();
    IMG_Quit();
    SDL_Quit();
}