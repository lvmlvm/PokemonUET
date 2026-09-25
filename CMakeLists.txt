cmake_minimum_required(VERSION 3.16)
project(PokemonUET LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

option(POKEMON_SANITIZE "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)

file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_executable(pokemon_uet WIN32 ${GAME_SOURCES})
target_include_directories(pokemon_uet PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(pokemon_uet PRIVATE
    $<$<TARGET_EXISTS:SDL2::SDL2main>:SDL2::SDL2main>
    SDL2::SDL2
    SDL2_image::SDL2_image
    SDL2_ttf::SDL2_ttf
    SDL2_mixer::SDL2_mixer
)

if(MSVC)
    target_compile_options(pokemon_uet PRIVATE /W4 /utf-8)
else()
    target_compile_options(pokemon_uet PRIVATE -Wall -Wextra)
endif()

if(POKEMON_SANITIZE AND NOT MSVC)
    target_compile_options(pokemon_uet PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(pokemon_uet PRIVATE -fsanitize=address,undefined)
endif()

# The game currently loads "res/..." and "data/..." relative to the working directory,
# so the runtime layout next to the executable is: res -> assets, data/ for saves.
set(RUNTIME_DIR $<TARGET_FILE_DIR:pokemon_uet>)
if(WIN32)
    add_custom_command(TARGET pokemon_uet POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${RUNTIME_DIR}/res
        COMMAND ${CMAKE_COMMAND} -E make_directory ${RUNTIME_DIR}/data)
else()
    add_custom_command(TARGET pokemon_uet POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/assets ${RUNTIME_DIR}/res
        COMMAND ${CMAKE_COMMAND} -E make_directory ${RUNTIME_DIR}/data)
endif()
