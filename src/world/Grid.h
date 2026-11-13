#pragma once
#include <vector>
#include <raylib.h>
#include <algorithm>
#include <cmath>

class Grid {
public:
    // We use a lower resolution grid than the screen for performance
    // e.g., 100x100 grid for an 800x450 screen
    Grid(int width, int height);
    ~Grid() = default;

    // Core Logic
    void update(float dt); // Decays danger over time
    void addDanger(float x, float y, float amount, float radius);
    float getDanger(float x, float y) const;

    // Visualization
    void draw(int screenWidth, int screenHeight) const;

    int getWidth() const { return width; }
    int getHeight() const { return height; }

private:
    int width, height;
    std::vector<float> dangerMap; // 1D array for cache locality!

    // Helper to convert 2D coordinates to 1D index
    inline int getIndex(int x, int y) const {
        // Clamp coordinates to prevent out-of-bounds
        x = std::max(0, std::min(x, width - 1));
        y = std::max(0, std::min(y, height - 1));
        return (y * width) + x;
    }
};