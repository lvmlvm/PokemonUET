#pragma once

#include "core/Sdl.h"

#include <map>
#include <string>
#include <utility>

// Loads game assets from the res/ folder next to the executable (falling back to ./res/),
// caching what can be shared. A missing image is logged once and replaced by a visible
// placeholder, so the game keeps running instead of dereferencing null.
class Assets {
public:
    Assets(SDL_Renderer* renderer, std::string root);
    Assets(const Assets&) = delete;
    Assets& operator=(const Assets&) = delete;

    static std::string findRoot();

    SDL_Renderer* renderer() const { return renderer_; }
    std::string path(const std::string& relative) const { return root_ + relative; }

    // Shared, cached textures: never change their color/alpha modulation or blend mode.
    SDL_Texture* texture(const std::string& relative);
    SDL_Texture* spriteSheet(const std::string& relative); // cyan (0,255,255) is transparent

    // A private copy the caller owns and may modulate freely.
    TexturePtr loadTexture(const std::string& relative);

    TTF_Font* font(int size); // the game font, cached per size
    Mix_Chunk* sound(const std::string& relative);

private:
    TexturePtr load(const std::string& relative, bool colorKeyed);
    TexturePtr placeholder();

    SDL_Renderer* renderer_;
    std::string root_;
    std::map<std::pair<std::string, bool>, TexturePtr> textures_;
    std::map<int, FontPtr> fonts_;
    std::map<std::string, ChunkPtr> sounds_;
};
