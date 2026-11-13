#include "Grid.h"

Grid::Grid(int width, int height) : width(width), height(height) {
    // Initialize the 1D vector with 0.0f
    dangerMap.resize(width * height, 0.0f);
}

float Grid::getDanger(float x, float y) const {
    // Convert screen coordinates to grid coordinates
    // Assuming grid maps 1:1 to a specific area, we'll just cast to int for now
    return dangerMap[getIndex((int)x, (int)y)];
}

void Grid::addDanger(float x, float y, float amount, float radius) {
    int startX = std::max(0, (int)(x - radius));
    int endX = std::min(width - 1, (int)(x + radius));
    int startY = std::max(0, (int)(y - radius));
    int endY = std::min(height - 1, (int)(y + radius));

    // Paint a circle of danger onto the grid
    for (int cy = startY; cy <= endY; ++cy) {
        for (int cx = startX; cx <= endX; ++cx) {
            float dist = std::hypot(cx - x, cy - y);
            if (dist <= radius) {
                int idx = getIndex(cx, cy);
                // Add danger, but don't exceed 1.0
                float falloff = amount * (1.0f - (dist / radius));
                dangerMap[idx] = std::min(1.0f, dangerMap[idx] + falloff);
            }
        }
    }
}

void Grid::update(float dt) {
    // Cellular Automata: Simple Decay
    // Every tick, danger fades away slightly
    for (float &val : dangerMap) {
        val *= 0.98f; // Decay factor (adjust to make danger last longer/shorter)
        if (val < 0.01f) val = 0.0f; // Clean up tiny floats
    }
}

void Grid::draw(int screenWidth, int screenHeight) const {
    // Calculate how many screen pixels each grid cell takes up
    float cellWidth = (float)screenWidth / width;
    float cellHeight = (float)screenHeight / height;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float danger = dangerMap[getIndex(x, y)];
            if (danger > 0.05f) {
                // Draw a semi-transparent red rectangle for danger
                // Color alpha is based on danger level
                Color c = { 255, 0, 0, (unsigned char)(danger * 100) };
                DrawRectangle(x * cellWidth, y * cellHeight, cellWidth + 1, cellHeight + 1, c);
            }
        }
    }
}