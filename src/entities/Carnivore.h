#pragma once
#include "Organism.h"
#include <raymath.h>

class Carnivore : public Organism {
public:
    Carnivore(float x, float y);
    void update(float dt, Grid& grid, const Quadtree& tree, std::vector<Organism*>& newEntities) override;
    void draw() const override;

private:
    Vector2 velocity;
    Vector2 acceleration;
    float maxForce;
    float changeDirectionTimer;
    float stalkTimer;
    float burstTimer;
    bool isBursting;

    // Radii
    float chaseRadius;   // Active pursuit distance
    float catchRadius;   // Biting distance

    // Energy thresholds (Your new scaled values)
    const float CRITICAL_ENERGY = 60.0f;   // Panic threshold
    const float REPRODUCE_ENERGY = 175.0f; // Very hard to reach
    const float COMFORT_ENERGY = 100.0f;
    const float BURST_COST = 10.0f;
    const float MAX_ENERGY = 250.0f;       // Add this!
};