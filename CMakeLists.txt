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
option(POKEMON_BUILD_TESTS "Build the unit tests (downloads the doctest header)" ON)

function(pokemon_compile_options target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /utf-8)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra)
    endif()
    if(POKEMON_SANITIZE AND NOT MSVC)
        target_compile_options(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(${target} PRIVATE -fsanitize=address,undefined)
    endif()
endfunction()

# Game rules and data, without SDL: battles, saves, the player, map files.
set(CORE_SOURCES
    src/battle/BattleEngine.cpp
    src/battle/Pokemon.cpp
    src/battle/PokemonData.cpp
    src/core/Rng.cpp
    src/save/SaveGame.cpp
    src/world/MapData.cpp
    src/world/MapInfo.cpp
    src/world/Player.cpp
)
add_library(pokemon_core STATIC ${CORE_SOURCES})
target_include_directories(pokemon_core PUBLIC ${CMAKE_SOURCE_DIR}/src)
pokemon_compile_options(pokemon_core)

# The game.
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)

file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)
foreach(core_source ${CORE_SOURCES})
    list(REMOVE_ITEM GAME_SOURCES ${CMAKE_SOURCE_DIR}/${core_source})
endforeach()

add_executable(pokemon_uet WIN32 ${GAME_SOURCES})
target_link_libraries(pokemon_uet PRIVATE
    pokemon_core
    $<$<TARGET_EXISTS:SDL2::SDL2main>:SDL2::SDL2main>
    SDL2::SDL2
    SDL2_image::SDL2_image
    SDL2_ttf::SDL2_ttf
    SDL2_mixer::SDL2_mixer
)
pokemon_compile_options(pokemon_uet)

# The game loads its assets from res/ next to the executable.
set(RUNTIME_DIR $<TARGET_FILE_DIR:pokemon_uet>)
if(WIN32)
    add_custom_command(TARGET pokemon_uet POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${RUNTIME_DIR}/res)
else()
    add_custom_command(TARGET pokemon_uet POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/assets ${RUNTIME_DIR}/res)
endif()

# Unit tests (no SDL needed): ctest --test-dir build
if(POKEMON_BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(doctest
        URL https://github.com/doctest/doctest/archive/refs/tags/v2.4.11.tar.gz
        URL_HASH SHA256=632ed2c05a7f53fa961381497bf8069093f0d6628c5f26286161fbd32a560186
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        SOURCE_SUBDIR doctest) # only the header is needed; this folder has no CMakeLists.txt
    FetchContent_MakeAvailable(doctest)

    enable_testing()
    file(GLOB TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/tests/*.cpp)
    add_executable(pokemon_tests ${TEST_SOURCES})
    target_include_directories(pokemon_tests PRIVATE ${doctest_SOURCE_DIR})
    target_link_libraries(pokemon_tests PRIVATE pokemon_core)
    target_compile_definitions(pokemon_tests PRIVATE POKEMON_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets/")
    pokemon_compile_options(pokemon_tests)
    add_test(NAME pokemon_tests COMMAND pokemon_tests)
endif()
