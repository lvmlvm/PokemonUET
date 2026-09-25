#!/usr/bin/env bash
# Builds and runs the game. Requires CMake and SDL2 (+ image, ttf, mixer):
#   macOS:  brew install cmake sdl2 sdl2_image sdl2_ttf sdl2_mixer
#   Ubuntu: sudo apt install cmake libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev
set -e
cd "$(dirname "$0")"

cmake -S . -B build "$@"
cmake --build build --parallel

cd build
./pokemon_uet
