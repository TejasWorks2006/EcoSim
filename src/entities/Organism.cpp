#include "Organism.h"

Organism::Organism(float x, float y, OrganismType type)
    : x(x), y(y), type(type), energy(100.0f), maxSpeed(50.0f), visionRadius(50.0f) {
}