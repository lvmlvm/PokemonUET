#include "core/Replay.h"

#include <SDL.h>
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

struct Command {
    Uint32 at;
    std::string op;
    std::string arg;
    int x = 0, y = 0;
};

std::vector<Command> commands;
size_t next = 0;
Uint32 frame = 0;
const Uint32 WATCHDOG_FRAMES = 600;

void pushKey(Uint32 type, const std::string& name) {
    SDL_Event ev{};
    ev.type = type;
    ev.key.state = type == SDL_KEYDOWN ? SDL_PRESSED : SDL_RELEASED;
    ev.key.keysym.sym = SDL_GetKeyFromName(name.c_str());
    ev.key.keysym.scancode = SDL_GetScancodeFromKey(ev.key.keysym.sym);
    SDL_PushEvent(&ev);
}

void pushClick(int x, int y) {
    SDL_Event ev{};
    ev.type = SDL_MOUSEMOTION;
    ev.motion.x = x;
    ev.motion.y = y;
    SDL_PushEvent(&ev);

    ev = SDL_Event{};
    ev.type = SDL_MOUSEBUTTONDOWN;
    ev.button.button = SDL_BUTTON_LEFT;
    ev.button.state = SDL_PRESSED;
    ev.button.x = x;
    ev.button.y = y;
    SDL_PushEvent(&ev);

    ev.type = SDL_MOUSEBUTTONUP;
    ev.button.state = SDL_RELEASED;
    SDL_PushEvent(&ev);
}

void screenshot(SDL_Renderer* renderer, const std::string& path) {
    int w = 0, h = 0;
    SDL_GetRendererOutputSize(renderer, &w, &h);
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ARGB8888);
    if (surface == nullptr) return;
    if (SDL_RenderReadPixels(renderer, nullptr, SDL_PIXELFORMAT_ARGB8888, surface->pixels, surface->pitch) != 0 ||
        SDL_SaveBMP(surface, path.c_str()) != 0) {
        std::cerr << "replay: screenshot failed: " << SDL_GetError() << '\n';
    }
    SDL_FreeSurface(surface);
}

} // namespace

void replay::load() {
    const char* path = std::getenv("POKEMON_REPLAY");
    if (path == nullptr) return;

    std::ifstream in(path);
    if (!in) {
        std::cerr << "replay: cannot open " << path << '\n';
        return;
    }

    std::string line;
    while (std::getline(in, line)) {
        line = line.substr(0, line.find('#'));
        std::istringstream words(line);
        Command cmd;
        if (!(words >> cmd.at >> cmd.op)) continue;
        if (cmd.op == "click") words >> cmd.x >> cmd.y;
        else words >> cmd.arg;

        if (cmd.op == "tap") {
            commands.push_back({cmd.at, "down", cmd.arg});
            commands.push_back({cmd.at + 5, "up", cmd.arg});
        } else {
            commands.push_back(cmd);
        }
    }
    std::stable_sort(commands.begin(), commands.end(), [](const Command& a, const Command& b) { return a.at < b.at; });
    std::cerr << "replay: loaded " << commands.size() << " commands from " << path << '\n';
}

double replay::speed() {
    static const double value = [] {
        const char* setting = std::getenv("POKEMON_REPLAY_SPEED");
        return setting != nullptr ? std::atof(setting) : 1.0;
    }();
    return value;
}

void replay::beforePresent(SDL_Renderer* renderer) {
    const Uint32 now = frame++;
    while (next < commands.size() && commands[next].at <= now) {
        const Command& cmd = commands[next++];
        std::cerr << "replay: frame " << now << ' ' << cmd.op << ' ' << cmd.arg << '\n';
        if (cmd.op == "down") pushKey(SDL_KEYDOWN, cmd.arg);
        else if (cmd.op == "up") pushKey(SDL_KEYUP, cmd.arg);
        else if (cmd.op == "click") pushClick(cmd.x, cmd.y);
        else if (cmd.op == "shot") screenshot(renderer, cmd.arg);
        else if (cmd.op == "quit") {
            SDL_Event ev{};
            ev.type = SDL_QUIT;
            SDL_PushEvent(&ev);
        }
    }

    if (!commands.empty() && next == commands.size()) {
        const Uint32 last = commands.back().at;
        if (now == last + WATCHDOG_FRAMES) {
            std::cerr << "replay: watchdog: script finished " << WATCHDOG_FRAMES << " frames ago, sending quit\n";
            SDL_Event ev{};
            ev.type = SDL_QUIT;
            SDL_PushEvent(&ev);
        } else if (now >= last + 2 * WATCHDOG_FRAMES) {
            std::cerr << "replay: watchdog: game ignored quit, aborting\n";
            std::_Exit(3);
        }
    }
}
