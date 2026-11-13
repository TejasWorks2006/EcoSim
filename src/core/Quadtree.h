#pragma once
#include <vector>
#include "raylib.h"
#include "../entities/Organism.h"

class Quadtree {
public:
    Quadtree(Rectangle boundary, int capacity);
    ~Quadtree();

    void clear();
    bool insert(Organism* p);
    void queryRange(Rectangle range, std::vector<Organism*>& found) const;

private:
    Rectangle boundary;
    int capacity;
    std::vector<Organism*> points;
    bool divided;

    Quadtree* northWest;
    Quadtree* northEast;
    Quadtree* southWest;
    Quadtree* southEast;

    void subdivide();
    bool inRange(Rectangle range, Organism* p) const;
};