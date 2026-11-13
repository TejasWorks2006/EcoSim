#pragma once
#include "Organism.h"
#include <raymath.h>

class Herbivore : public Organism {
public:
    Herbivore(float x, float y);
    void update(float dt, Grid& grid, const Quadtree& tree, std::vector<Organism*>& newEntities) override;
    void draw() const override;

private:
    Vector2 velocity;
    Vector2 acceleration;
    float maxForce;
    float changeDirectionTimer;

    // Herding
    Vector2 herdCenter;
    int nearbyHerdCount;

    // Energy thresholds (Optimized for stable population growth)
    const float CRITICAL_ENERGY = 20.0f;
    const float REPRODUCE_ENERGY = 100.0f; // Harder to reach
    const float MAX_ENERGY = 120.0f;       // Cap to prevent hoarding
};