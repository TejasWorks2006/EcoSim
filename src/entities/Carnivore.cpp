#include "Carnivore.h"
#include "../core/Quadtree.h"
#include "../world/Grid.h"
#include <cstdlib>
#include <cmath>
#include <algorithm> // Required for std::clamp

Carnivore::Carnivore(float x, float y)
    : Organism(x, y, OrganismType::Carnivore),
      changeDirectionTimer(0.0f),
      burstTimer(0.0f),
      isBursting(false),
      chaseRadius(100.0f),   // REDUCED: Harder to lock on from far away
      catchRadius(12.0f) {   // REDUCED: Much smaller bite hitbox (Low Accuracy)
    maxSpeed = 60.0f;        // REDUCED from 90.0f
    maxForce = 300.0f;
    visionRadius = 180.0f;   // REDUCED: Harder to spot prey
    energy = 120.0f;
    velocity = {0, 0};
    acceleration = {0, 0};
    state = State::RELOCATING;
}

void Carnivore::update(float dt, Grid& grid, const Quadtree& tree, std::vector<Organism*>& newEntities) {
    grid.addDanger(x, y, 0.3f, 20.0f);

    Rectangle senseZone = {x - visionRadius, y - visionRadius, visionRadius*2, visionRadius*2};
    std::vector<Organism*> nearby;
    tree.queryRange(senseZone, nearby);

    Organism* targetPrey = nullptr;
    float minPreyDist = 99999.0f;
    Organism* mateTarget = nullptr;
    float minMateDist = 99999.0f;

    for (Organism* other : nearby) {
        if (!other->isAlive()) continue;
        float dist = std::hypot(x - other->getX(), y - other->getY());

        if (other->getType() == OrganismType::Herbivore) {
            if (dist < minPreyDist) { minPreyDist = dist; targetPrey = other; }
        } else if (other->getType() == OrganismType::Carnivore && other != this) {
            if (dist < minMateDist) { minMateDist = dist; mateTarget = other; }
        }
    }

    // === BEHAVIOR LOGIC ===

    // Priority 1: REPRODUCE
    if (energy >= 200.0f) {
        state = State::REPRODUCING;
        isBursting = false;

        if (mateTarget != nullptr && minMateDist < 30.0f) {
            velocity = {0, 0};
            energy -= 90.0f;
            float offsetX = (rand() % 30) - 15;
            float offsetY = (rand() % 30) - 15;
            float spawnX = std::clamp(x + offsetX, 50.0f, (float)WORLD_WIDTH - 50.0f);
            float spawnY = std::clamp(y + offsetY, 50.0f, (float)WORLD_HEIGHT - 50.0f);
            newEntities.push_back(new Carnivore(spawnX, spawnY));
        } else if (mateTarget != nullptr) {
            float dx = mateTarget->getX() - x;
            float dy = mateTarget->getY() - y;
            float len = std::hypot(dx, dy);
            velocity.x = (dx / len) * (maxSpeed * 0.5f);
            velocity.y = (dy / len) * (maxSpeed * 0.5f);
        } else {
            changeDirectionTimer -= dt;
            if (changeDirectionTimer <= 0.0f) {
                float angle = (rand() % 360) * DEG2RAD;
                velocity.x = cos(angle) * (maxSpeed * 0.4f);
                velocity.y = sin(angle) * (maxSpeed * 0.4f);
                changeDirectionTimer = 2.0f + (rand() % 200) / 100.0f;
            }
        }
    }
    // Priority 2: CONSERVE ENERGY (Full)
    else if (energy >= 160.0f) { // COMFORT_ENERGY
        state = State::RELOCATING;
        isBursting = false;
        changeDirectionTimer -= dt;
        if (changeDirectionTimer <= 0.0f) {
            float angle = (rand() % 360) * DEG2RAD;
            velocity.x = cos(angle) * (maxSpeed * 0.2f);
            velocity.y = sin(angle) * (maxSpeed * 0.2f);
            changeDirectionTimer = 4.0f + (rand() % 200) / 100.0f;
        }
    }
    // Priority 3: ACTIVE HUNT (Low Accuracy)
    else if (targetPrey != nullptr && minPreyDist < chaseRadius) {
        state = State::HUNTING;
        // Burst only when very close to try and secure the difficult hitbox
        isBursting = (minPreyDist < 40.0f);

        float dx = targetPrey->getX() - x;
        float dy = targetPrey->getY() - y;
        float len = std::hypot(dx, dy);

        // REDUCED multipliers. Max chase speed is now 75.0f (Herbivore max flee is 54.0f)
        float speedMultiplier = isBursting ? 1.25f : 1.15f;
        velocity.x = (dx / len) * (maxSpeed * speedMultiplier);
        velocity.y = (dy / len) * (maxSpeed * speedMultiplier);

        // INSTANT KILL (Only if perfectly overlapping the tiny 12px hitbox)
        if (minPreyDist < catchRadius) {
            energy = std::min(250.0f, energy + 80.0f);
            targetPrey->setEnergy(-100.0f);
        }
    }
    // Priority 4: RELOCATE / SEARCH
    else {
        state = State::RELOCATING;
        isBursting = false;
        changeDirectionTimer -= dt;
        if (changeDirectionTimer <= 0.0f) {
            float angle = (rand() % 360) * DEG2RAD;
            velocity.x = cos(angle) * (maxSpeed * 0.4f);
            velocity.y = sin(angle) * (maxSpeed * 0.4f);
            changeDirectionTimer = 2.0f + (rand() % 200) / 100.0f;
        }
    }

    // === WALL REPULSION ===
    float margin = 20.0f;
    float repulsionStrength = 500.0f;
    if (x < margin) acceleration.x += repulsionStrength;
    if (x > WORLD_WIDTH - margin) acceleration.x -= repulsionStrength;
    if (y < margin) acceleration.y += repulsionStrength;
    if (y > WORLD_HEIGHT - margin) acceleration.y -= repulsionStrength;

    // Apply physics
    velocity.x += acceleration.x * dt;
    velocity.y += acceleration.y * dt;
    acceleration = {0, 0};

    float maxVel = isBursting ? maxSpeed * 1.25f : maxSpeed;
    float velLen = std::hypot(velocity.x, velocity.y);
    if (velLen > maxVel) {
        velocity.x = (velocity.x / velLen) * maxVel;
        velocity.y = (velocity.y / velLen) * maxVel;
    }

    x += velocity.x * dt;
    y += velocity.y * dt;
    x = std::max(0.0f, std::min((float)WORLD_WIDTH, x));
    y = std::max(0.0f, std::min((float)WORLD_HEIGHT, y));

    // === METABOLISM ===
    if (state == State::HUNTING && isBursting) energy -= 6.0f * dt;
    else if (energy >= 160.0f) energy -= 1.0f * dt;
    else energy -= 2.5f * dt;
}

void Carnivore::draw() const {
    Color c = RED;
    float radius = 5.0f;

    if (state == State::HUNTING) {
        c = isBursting ? ORANGE : MAGENTA;
        radius = 6.0f;
    } else if (state == State::REPRODUCING) {
        c = PINK;
    } else if (energy >= 160.0f) {
        c = Fade(DARKGREEN, 0.6f);
        radius = 5.5f;
    } else {
        c = Fade(RED, 0.5f);
    }

    DrawCircle(x, y, radius, c);
}