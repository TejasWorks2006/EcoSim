#include "Simulation.h"
#include "../entities/Autotroph.h"
#include "../entities/Herbivore.h"
#include "../entities/Carnivore.h"
#include "../core/Quadtree.h"
#include <iostream>
#include <cmath>
#include <string>

Simulation::Simulation() : grid(100, 75) { // UPDATED: 800/8 = 100, 600/8 = 75 (Maintains cell size)
    spawnInitialPopulation();
}

Simulation::~Simulation() {
    for (Organism* org : organisms) {
        delete org;
    }
    organisms.clear();
}

void Simulation::spawnInitialPopulation() {
    const float SPAWN_MARGIN = 50.0f;
    int safeWidth = WORLD_WIDTH - (2 * SPAWN_MARGIN);
    int safeHeight = WORLD_HEIGHT - (2 * SPAWN_MARGIN);

    // Scaled down populations to fit the new 800x600 map without instant overcrowding
    for (int i = 0; i < 300; ++i)
        organisms.push_back(new Autotroph(SPAWN_MARGIN + (rand() % safeWidth), SPAWN_MARGIN + (rand() % safeHeight)));

    for (int i = 0; i < 60; ++i)
        organisms.push_back(new Herbivore(SPAWN_MARGIN + (rand() % safeWidth), SPAWN_MARGIN + (rand() % safeHeight)));

    for (int i = 0; i < 5; ++i)
        organisms.push_back(new Carnivore(SPAWN_MARGIN + (rand() % safeWidth), SPAWN_MARGIN + (rand() % safeHeight)));
}

void Simulation::update(float dt) {
    grid.update(dt);

    // 1. Build the Quadtree for this frame
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

    // 3. Add babies with individual population caps
    int autotrophCount = 0;
    int herbivoreCount = 0;
    int carnivoreCount = 0;

    for (Organism* org : organisms) {
        if (org->getType() == OrganismType::Autotroph) autotrophCount++;
        else if (org->getType() == OrganismType::Herbivore) herbivoreCount++;
        else if (org->getType() == OrganismType::Carnivore) carnivoreCount++;
    }

    for (Organism* baby : newEntities) {
        bool underLimit = false;

        if (baby->getType() == OrganismType::Autotroph && autotrophCount < MAX_AUTOTROPHS) {
            autotrophCount++;
            underLimit = true;
        }
        else if (baby->getType() == OrganismType::Herbivore && herbivoreCount < MAX_HERBIVORES) {
            herbivoreCount++;
            underLimit = true;
        }
        else if (baby->getType() == OrganismType::Carnivore && carnivoreCount < MAX_CARNIVORES) {
            carnivoreCount++;
            underLimit = true;
        }

        if (underLimit) {
            organisms.push_back(baby);
        } else {
            delete baby; // CRITICAL: Prevent memory leaks by deleting culled babies
        }
    }

    // 4. Cleanup dead organisms (O(1) Swap-and-Pop)
    for (size_t i = 0; i < organisms.size(); ) {
        if (organisms[i]->getEnergy() <= 0.0f) {
            delete organisms[i];
            organisms[i] = organisms.back();
            organisms.pop_back();
        } else {
            ++i;
        }
    }
}

void Simulation::draw() const {
    grid.draw(WORLD_WIDTH, WORLD_HEIGHT);

    for (const Organism* org : organisms) {
        org->draw();
    }

    DrawText(TextFormat("Population: %d | FPS: %d", (int)organisms.size(), GetFPS()), 10, 10, 20, WHITE);
}

// --- NEW FUNCTION: Check for Extinction ---
bool Simulation::checkExtinction(std::string &extinctSpecies) const {
    int autoCount = 0, herbCount = 0, carnCount = 0;

    for (const Organism* org : organisms) {
        if (org->getType() == OrganismType::Autotroph) autoCount++;
        else if (org->getType() == OrganismType::Herbivore) herbCount++;
        else if (org->getType() == OrganismType::Carnivore) carnCount++;
    }

    if (autoCount == 0) { extinctSpecies = "Autotrophs (Plants)"; return true; }
    if (herbCount == 0) { extinctSpecies = "Herbivores"; return true; }
    if (carnCount == 0) { extinctSpecies = "Carnivores"; return true; }

    return false;
}