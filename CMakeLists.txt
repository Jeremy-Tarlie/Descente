cmake_minimum_required(VERSION 3.16)
project(Descente VERSION 1.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/W4> $<$<COMPILE_LANGUAGE:CXX>:/permissive->)
else()
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wall> $<$<COMPILE_LANGUAGE:CXX>:-Wextra> $<$<COMPILE_LANGUAGE:CXX>:-Wpedantic>)
endif()

# --- raylib (FetchContent) ----------------------------------------------------
include(FetchContent)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG        5.5
)
FetchContent_MakeAvailable(raylib)

# --- Core library (logic only — shared by game + tests) -----------------------
set(DESCENTE_CORE_SOURCES
  src/map/Map.cpp
  src/map/BspGenerator.cpp
  src/fov/Fov.cpp
  src/pathfinding/AStar.cpp
  src/game/SaveLoad.cpp
  src/game/Game.cpp
  src/systems/AISystem.cpp
  src/systems/CombatSystem.cpp
  src/systems/InventorySystem.cpp
)

add_library(descente_core STATIC ${DESCENTE_CORE_SOURCES})
target_include_directories(descente_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# --- Executable (UI + raylib) -------------------------------------------------
add_executable(descente
  src/main.cpp
  src/ui/SpriteAtlas.cpp
  src/ui/Renderer.cpp
  src/ui/InputPoll.cpp
)
target_include_directories(descente PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(descente PRIVATE descente_core raylib)

# --- Tests --------------------------------------------------------------------
enable_testing()
add_subdirectory(tests)
