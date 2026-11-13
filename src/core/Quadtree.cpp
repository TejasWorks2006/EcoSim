#include "Quadtree.h"
#include <cmath>

Quadtree::Quadtree(Rectangle boundary, int capacity)
    : boundary(boundary), capacity(capacity), divided(false),
      northWest(nullptr), northEast(nullptr), southWest(nullptr), southEast(nullptr) {}

Quadtree::~Quadtree() {
    delete northWest;
    delete northEast;
    delete southWest;
    delete southEast;
}

void Quadtree::clear() {
    points.clear();
    divided = false;
    delete northWest; northWest = nullptr;
    delete northEast; northEast = nullptr;
    delete southWest; southWest = nullptr;
    delete southEast; southEast = nullptr;
}

void Quadtree::subdivide() {
    float x = boundary.x;
    float y = boundary.y;
    float w = boundary.width / 2.0f;
    float h = boundary.height / 2.0f;

    Rectangle nw = { x, y, w, h };
    Rectangle ne = { x + w, y, w, h };
    Rectangle sw = { x, y + h, w, h };
    Rectangle se = { x + w, y + h, w, h };

    northWest = new Quadtree(nw, capacity);
    northEast = new Quadtree(ne, capacity);
    southWest = new Quadtree(sw, capacity);
    southEast = new Quadtree(se, capacity);

    divided = true;
}

bool Quadtree::insert(Organism* p) {
    if (!CheckCollisionPointRec({p->getX(), p->getY()}, boundary)) {
        return false;
    }

    if ((int)points.size() < capacity) {
        points.push_back(p);
        return true;
    }

    if (!divided) {
        subdivide();
    }

    if (northWest->insert(p)) return true;
    if (northEast->insert(p)) return true;
    if (southWest->insert(p)) return true;
    if (southEast->insert(p)) return true;

    return false;
}

bool Quadtree::inRange(Rectangle range, Organism* p) const {
    return CheckCollisionPointRec({p->getX(), p->getY()}, range);
}

void Quadtree::queryRange(Rectangle range, std::vector<Organism*>& found) const {
    if (!CheckCollisionRecs(boundary, range)) {
        return; // No overlap
    }

    for (Organism* p : points) {
        if (inRange(range, p)) {
            found.push_back(p);
        }
    }

    if (divided) {
        northWest->queryRange(range, found);
        northEast->queryRange(range, found);
        southWest->queryRange(range, found);
        southEast->queryRange(range, found);
    }
}