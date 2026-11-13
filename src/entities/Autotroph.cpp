#include "Autotroph.h"
#include <cstdlib>
#include <algorithm>

Autotroph::Autotroph(float x, float y)
    : Organism(x, y, OrganismType::Autotroph) {
    energy = 30.0f; // Start small
    state = State::WANDERING;
}

void Autotroph::update(float dt, Grid& grid, const Quadtree& tree, std::vector<Organism*>& newEntities) {
    // Steady, moderate growth
    if (energy < 100.0f) {
        energy += 4.0f * dt;
    }

    // Reproduce when mature (Splits energy in half)
    if (energy >= 90.0f) {
        energy -= 45.0f;

        float offsetX = (rand() % 30) - 15;
        float offsetY = (rand() % 30) - 15;

        // CLAMP: Guarantee the baby spawns at least 50px away from any edge
        float spawnX = std::clamp(x + offsetX, 50.0f, (float)WORLD_WIDTH - 50.0f);
        float spawnY = std::clamp(y + offsetY, 50.0f, (float)WORLD_HEIGHT - 50.0f);

        newEntities.push_back(new Autotroph(spawnX, spawnY));
    }
}

void Autotroph::draw() const {
    float radius = std::max(2.0f, energy / 15.0f);
    DrawCircle(x, y, radius, GREEN);
}