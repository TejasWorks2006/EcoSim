#include "raylib.h"
#include "src/world/Simulation.h"
#include "src/entities/Organism.h"
#include <iostream>
#include <string>

int main() {
    // 1. Initialization
    InitWindow(WORLD_WIDTH, WORLD_HEIGHT, "EcoSim - ADSA Project");
    SetTargetFPS(60);

    Simulation sim;

    // Start the timer
    double startTime = GetTime();
    bool simulationRunning = true;

    // 2. Main Game Loop
    while (!WindowShouldClose() && simulationRunning) {
        float dt = GetFrameTime();
        sim.update(dt);

        // --- EXTINCTION CHECK ---
        std::string extinctSpecies;
        if (sim.checkExtinction(extinctSpecies)) {
            simulationRunning = false;

            // Calculate final runtime
            double totalTime = GetTime() - startTime;
            int minutes = (int)totalTime / 60;
            int seconds = (int)totalTime % 60;

            // Print to console (Returns the runtime)
            std::cout << "=========================================" << std::endl;
            std::cout << "SIMULATION ENDED: " << extinctSpecies << " went extinct!" << std::endl;
            std::cout << "Total Runtime: " << minutes << " minutes and " << seconds << " seconds." << std::endl;
            std::cout << "=========================================" << std::endl;

            // Draw the final "Game Over" screen
            BeginDrawing();
            ClearBackground(BLACK);
            DrawText(TextFormat("EXTINCTION EVENT: %s", extinctSpecies.c_str()), 20, 20, 30, RED);
            DrawText(TextFormat("Total Runtime: %d min %d sec", minutes, seconds), 20, 60, 30, WHITE);
            DrawText("Press ESC to exit", 20, 100, 20, GRAY);
            EndDrawing();

            // Pause and wait for the user to press ESC before closing
            while (!WindowShouldClose()) {
                if (IsKeyPressed(256)) break;
            }
            break;
        }

        // --- CALCULATE LIVE TIMER ---
        double currentTime = GetTime() - startTime;
        int mins = (int)currentTime / 60;
        int secs = (int)currentTime % 60;

        // --- DRAW ---
        BeginDrawing();
        ClearBackground(BLACK);

        sim.draw();

        // Draw Timer at the top center
        std::string timeStr = TextFormat("Runtime: %02d:%02d", mins, secs);
        int textWidth = MeasureText(timeStr.c_str(), 20);
        DrawText(timeStr.c_str(), (WORLD_WIDTH - textWidth) / 2, 10, 20, WHITE);

        EndDrawing();
    }

    // 3. De-Initialization
    CloseWindow();
    return 0;
}