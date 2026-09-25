#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

class RenderWindow {
    public:
        RenderWindow();
        ~RenderWindow();
        SDL_Texture* loadTexture(const char* path);
        void create(const char *title = "Title", int w = 64*13, int h = 64*11);
        void close();
        void clear();
        void drawColor(int red, int green, int blue);
        void render(SDL_Texture* texture, SDL_Rect* clip = NULL);
        void display();

        static SDL_Renderer* renderer;
        
    private:
        SDL_Window* window;
};


// Mouse position carried by a mouse motion/button event (works for real and injected events).
inline void eventMousePosition(const SDL_Event& ev, int& x, int& y) {
    if (ev.type == SDL_MOUSEMOTION) { x = ev.motion.x; y = ev.motion.y; }
    else { x = ev.button.x; y = ev.button.y; }
}
