#include "Simulation.h"
#include "../entities/Autotroph.h"
#include "../entities/Herbivore.h"
#include "../entities/Carnivore.h"
#include "../core/Quadtree.h"
#include <iostream>
#include <cmath>

Simulation::Simulation() : grid(150, 100) { // 1200/8 = 150, 800/8 = 100 (Maintains cell size)
    spawnInitialPopulation();
}

Simulation::~Simulation() {
    for (Organism* org : organisms) {
        delete org;
    }
    organisms.clear();
}

void Simulation::spawnInitialPopulation() {
    // Scaled up populations to fill the larger map
    for (int i = 0; i < 500; ++i) organisms.push_back(new Autotroph(rand() % WORLD_WIDTH, rand() % WORLD_HEIGHT));
    for (int i = 0; i < 80; ++i)  organisms.push_back(new Herbivore(rand() % WORLD_WIDTH, rand() % WORLD_HEIGHT));
    for (int i = 0; i < 6; ++i)   organisms.push_back(new Carnivore(rand() % WORLD_WIDTH, rand() % WORLD_HEIGHT));
}

void Simulation::update(float dt) {
    grid.update(dt);

    // 1. Build the Quadtree for this frame (UPDATED BOUNDS)
    Rectangle worldBounds = {0, 0, (float)WORLD_WIDTH, (float)WORLD_HEIGHT};
    Quadtree tree(worldBounds, 4);

    for (Organism* org : organisms) {
        if (org->isAlive()) {
            tree.insert(org);
        }
    }

    // 2. Update Organisms
    std::vector<Organism*> newEntities;
    for (Organism* org : organisms) {
        if (org->isAlive()) {
            org->update(dt, grid, tree, newEntities);
        }
    }
    
    // 3. Add babies (No hard limit! The environment will naturally cull them via starvation)
    for (Organism* baby : newEntities) {
        organisms.push_back(baby);
    }

    // 4. Cleanup dead (O(1) Swap-and-Pop)
    for (size_t i = 0; i < organisms.size(); ) {
        if (!organisms[i]->isAlive()) {
            delete organisms[i];
            organisms[i] = organisms.back();
            organisms.pop_back();
        } else {
            ++i;
        }
    }
}

void Simulation::draw() const {
    // UPDATED: Pass new dimensions to grid draw
    grid.draw(WORLD_WIDTH, WORLD_HEIGHT);

    for (const Organism* org : organisms) {
        org->draw();
    }

    DrawText(TextFormat("Population: %d | FPS: %d", (int)organisms.size(), GetFPS()), 10, 10, 20, WHITE);
}