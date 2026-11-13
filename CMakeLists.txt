cmake_minimum_required(VERSION 3.11)
project(EcoSim)

set(CMAKE_CXX_STANDARD 17)

# --- FIX: Suppress Warnings ---
set(DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# --- CRITICAL: Stop Raylib from building examples ---
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

# --- Download Raylib ---
include(FetchContent)
FetchContent_Declare(
        raylib
        URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz
)
FetchContent_MakeAvailable(raylib)

# --- YOUR PROJECT SETUP ---
# Collect ALL .cpp files from src/ recursively
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")

add_executable(EcoSim main.cpp ${SOURCES})

# Link Raylib to your executable
target_link_libraries(EcoSim raylib)