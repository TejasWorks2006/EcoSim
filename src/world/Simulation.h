#pragma once
#include "../entities/Organism.h"
#include "Grid.h"
#include <vector>

const int MAX_POPULATION = 4000;

class Simulation {
public:
    Simulation();
    ~Simulation();

    void update(float dt);
    void draw() const;

private:
    Grid grid;
    std::vector<Organism*> organisms;

    void spawnInitialPopulation();
};