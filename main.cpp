#include "raylib.h"
#include "src/world/Simulation.h"
#include "src/entities/Organism.h" // To access WORLD_WIDTH/HEIGHT
#include <iostream>

int main() {
    // 1. Initialization (UPDATED)
    InitWindow(WORLD_WIDTH, WORLD_HEIGHT, "EcoSim - ADSA Project");
    SetTargetFPS(60);

    Simulation sim;

    // 2. Main Game Loop
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // --- UPDATE ---
        sim.update(dt);

        // --- DRAW ---
        BeginDrawing();
        ClearBackground(BLACK);

        sim.draw();

        EndDrawing();
    }

    // 3. De-Initialization
    CloseWindow();
    return 0;
}