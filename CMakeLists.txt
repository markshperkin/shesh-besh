cmake_minimum_required(VERSION 3.20)
project(Backgammon LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20) # request C++ 20 for all targets
set(CMAKE_CXX_STANDARD_REQUIRED ON) # no fall back to older standard if the compiler cant do C++ 20

# Backgammon rules library
add_library(backgammon 
    src/engine/vanilla_rules_engine.cpp
    src/engine/board.cpp
    src/engine/dice.cpp
    src/game/game.cpp
    src/game/match_session.cpp
    src/game/match_maker.cpp
    src/players/rbot.cpp
)

target_include_directories(backgammon
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

enable_testing() # google tests
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
  )
FetchContent_MakeAvailable(googletest)

# Applications

add_subdirectory(tests)
# Qt client
# add_subdirectory(apps/SheshBesh)
