#include "core/Assets.h"

#include <fstream>

namespace {
const char* FONT_PATH = "font/gamefont.ttf";

bool exists(const std::string& path) {
    return std::ifstream(path).good();
}
} // namespace

Assets::Assets(SDL_Renderer* renderer, std::string root)
    : renderer_(renderer), root_(std::move(root)) {}

std::string Assets::findRoot() {
    if (char* base = SDL_GetBasePath()) {
        std::string root = std::string(base) + "res/";
        SDL_free(base);
        if (exists(root + FONT_PATH)) return root;
    }
    return "res/";
}

SDL_Texture* Assets::texture(const std::string& relative) {
    TexturePtr& slot = textures_[{relative, false}];
    if (!slot) slot = load(relative, false);
    return slot.get();
}

SDL_Texture* Assets::spriteSheet(const std::string& relative) {
    TexturePtr& slot = textures_[{relative, true}];
    if (!slot) slot = load(relative, true);
    return slot.get();
}

TexturePtr Assets::loadTexture(const std::string& relative) {
    return load(relative, false);
}

TexturePtr Assets::load(const std::string& relative, bool colorKeyed) {
    SurfacePtr surface(IMG_Load(path(relative).c_str()));
    if (!surface) {
        SDL_Log("Failed to load image %s: %s", path(relative).c_str(), IMG_GetError());
        return placeholder();
    }
    if (colorKeyed) SDL_SetColorKey(surface.get(), SDL_TRUE, SDL_MapRGB(surface->format, 0, 255, 255));
    return TexturePtr(SDL_CreateTextureFromSurface(renderer_, surface.get()));
}

TexturePtr Assets::placeholder() {
    SurfacePtr surface(SDL_CreateRGBSurfaceWithFormat(0, 16, 16, 32, SDL_PIXELFORMAT_ARGB8888));
    if (!surface) return nullptr;
    SDL_FillRect(surface.get(), nullptr, SDL_MapRGB(surface->format, 255, 0, 255));
    return TexturePtr(SDL_CreateTextureFromSurface(renderer_, surface.get()));
}

TTF_Font* Assets::font(int size) {
    FontPtr& slot = fonts_[size];
    if (!slot) {
        slot.reset(TTF_OpenFont(path(FONT_PATH).c_str(), size));
        if (!slot) SDL_Log("Failed to load font %s: %s", path(FONT_PATH).c_str(), TTF_GetError());
    }
    return slot.get();
}

Mix_Chunk* Assets::sound(const std::string& relative) {
    ChunkPtr& slot = sounds_[relative];
    if (!slot) {
        slot.reset(Mix_LoadWAV(path(relative).c_str()));
        if (!slot) SDL_Log("Failed to load sound %s: %s", path(relative).c_str(), Mix_GetError());
    }
    return slot.get();
}
