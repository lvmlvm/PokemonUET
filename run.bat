@echo off
rem Builds and runs the game. Requires CMake and SDL2 (+ image, ttf, mixer), e.g. via vcpkg:
rem   vcpkg install sdl2 sdl2-image sdl2-ttf sdl2-mixer[mpg123]
rem   run.bat -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
cd /d "%~dp0"

cmake -S . -B build %*
if errorlevel 1 exit /b 1
cmake --build build --config Release --parallel
if errorlevel 1 exit /b 1

if exist build\Release\pokemon_uet.exe (
    cd build\Release
) else (
    cd build
)
pokemon_uet.exe
