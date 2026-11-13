#pragma once
#include <raylib.h>
#include <cstdint>
#include <vector>

const int WORLD_WIDTH = 1200;
const int WORLD_HEIGHT = 800;

class Grid;
class Quadtree;

enum class OrganismType { Autotroph, Herbivore, Carnivore, Scavenger };
enum class State {
    WANDERING,
    FORAGING,
    FLEEING,
    STALKING,
    HUNTING,
    RELOCATING,
    RESTING,        // Conserving energy
    REPRODUCING,    // Looking for mates
    DESPERATE_HUNT  // Carnivore last resort
};

class Organism {
public:
    Organism(float x, float y, OrganismType type);
    virtual ~Organism() = default;

    virtual void update(float dt, Grid& grid, const Quadtree& tree, std::vector<Organism*>& newEntities) = 0;
    virtual void draw() const = 0;

    float getX() const { return x; }
    float getY() const { return y; }
    OrganismType getType() const { return type; }
    float getEnergy() const { return energy; }
    bool isAlive() const { return energy > 0; }
    State getState() const { return state; }

    void setEnergy(float e) { energy = e; }
    void setPosition(float nx, float ny) { x = nx; y = ny; }
    void setState(State s) { state = s; }

protected:
    float x, y;
    float energy;
    OrganismType type;
    State state;

    float maxSpeed;
    float visionRadius;
};