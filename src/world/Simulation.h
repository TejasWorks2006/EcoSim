#pragma once
#include "../entities/Organism.h"
#include "Grid.h"
#include <vector>
#include <string> // <-- ADD THIS

// --- INDIVIDUAL POPULATION CAPS ---
const int MAX_AUTOTROPHS = 400; // Reduced for smaller map
const int MAX_HERBIVORES = 80;
const int MAX_CARNIVORES = 15;

class Simulation {
public:
    Simulation();
    ~Simulation();

    void update(float dt);
    void draw() const;

    // NEW: Check if any species has died out
    bool checkExtinction(std::string &extinctSpecies) const;

private:
    Grid grid;
    std::vector<Organism*> organisms;
    void spawnInitialPopulation();
};