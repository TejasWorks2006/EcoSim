#pragma once
#include "Organism.h"

class Autotroph : public Organism {
public:
    Autotroph(float x, float y);
    // UPDATED: Added const Quadtree& tree
    void update(float dt, Grid& grid, const Quadtree& tree, std::vector<Organism*>& newEntities) override;
    void draw() const override;

private:
    float growthRate;
    float reproduceThreshold;
};