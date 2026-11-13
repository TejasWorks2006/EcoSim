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
    float burstTimer;
    bool isBursting;

    // Radii
    float chaseRadius;   // Active pursuit distance
    float catchRadius;   // Biting distance

    // Energy thresholds
    const float CRITICAL_ENERGY = 60.0f;
    const float REPRODUCE_ENERGY = 350.0f;
    const float COMFORT_ENERGY = 300.0f;
    const float MAX_ENERGY = 400.0f;
    const float BURST_COST = 10.0f;
};