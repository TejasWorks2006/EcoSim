#include "Herbivore.h"
#include "../core/Quadtree.h"
#include <cstdlib>
#include <cmath>
#include <algorithm> // Required for std::max, std::min, std::clamp

Herbivore::Herbivore(float x, float y)
    : Organism(x, y, OrganismType::Herbivore),
      changeDirectionTimer(0.0f),
      herdCenter{0, 0},
      nearbyHerdCount(0) {
    maxSpeed = 47.25f;       // REDUCED from 65.0f
    maxForce = 200.0f;
    visionRadius = 150.0f;
    energy = 120.0f;
    velocity = {0, 0};
    acceleration = {0, 0};
    state = State::WANDERING;
}

void Herbivore::update(float dt, Grid& grid, const Quadtree& tree, std::vector<Organism*>& newEntities) {
    // 1. SENSE THE ENVIRONMENT
    Rectangle senseZone = {x - visionRadius, y - visionRadius, visionRadius*2, visionRadius*2};
    std::vector<Organism*> nearby;
    tree.queryRange(senseZone, nearby);

    Organism* nearestThreat = nullptr;
    float minThreatDist = 99999.0f;
    Organism* nearestFood = nullptr;
    float minFoodDist = 99999.0f;
    herdCenter = {0, 0};
    nearbyHerdCount = 0;
    Organism* nearestMate = nullptr;
    float minMateDist = 99999.0f;

    for (Organism* other : nearby) {
        if (other == this || !other->isAlive()) continue;
        float dx = other->getX() - x;
        float dy = other->getY() - y;
        float dist = std::hypot(dx, dy);

        if (other->getType() == OrganismType::Carnivore) {
            if (dist < minThreatDist) { minThreatDist = dist; nearestThreat = other; }
        } else if (other->getType() == OrganismType::Autotroph) {
            if (dist < minFoodDist) { minFoodDist = dist; nearestFood = other; }
        } else if (other->getType() == OrganismType::Herbivore) {
            herdCenter.x += other->getX();
            herdCenter.y += other->getY();
            nearbyHerdCount++;
            if (dist < minMateDist) { minMateDist = dist; nearestMate = other; }
        }
    }

    // 2. BEHAVIOR PRIORITY SYSTEM
    if (nearestThreat != nullptr && minThreatDist < 80.0f) {
        state = State::FLEEING;
        float dx = x - nearestThreat->getX();
        float dy = y - nearestThreat->getY();
        float len = std::hypot(dx, dy);
        if (len > 0) {
            // REDUCED flee multiplier to 1.2f
            velocity.x = (dx / len) * (maxSpeed * 1.2f);
            velocity.y = (dy / len) * (maxSpeed * 1.2f);
        }
    }
    else if (energy >= 180.0f) { // REPRODUCE_ENERGY
        state = State::REPRODUCING;
        if (nearbyHerdCount > 0) {
            herdCenter.x /= nearbyHerdCount;
            herdCenter.y /= nearbyHerdCount;
            float dx = herdCenter.x - x;
            float dy = herdCenter.y - y;
            float dist = std::hypot(dx, dy);

            if (dist > 10.0f) {
                velocity.x = (dx / dist) * (maxSpeed * 0.6f);
                velocity.y = (dy / dist) * (maxSpeed * 0.6f);
            } else {
                velocity = {0, 0};
                if (nearestMate != nullptr && minMateDist < 30.0f) {
                    energy -= 50.0f;
                    float offsetX = (rand() % 20) - 10;
                    float offsetY = (rand() % 20) - 10;
                    float spawnX = std::clamp(x + offsetX, 50.0f, (float)WORLD_WIDTH - 50.0f);
                    float spawnY = std::clamp(y + offsetY, 50.0f, (float)WORLD_HEIGHT - 50.0f);
                    newEntities.push_back(new Herbivore(spawnX, spawnY));
                }
            }
        } else {
            changeDirectionTimer -= dt;
            if (changeDirectionTimer <= 0.0f) {
                float angle = (rand() % 360) * DEG2RAD;
                velocity.x = cos(angle) * (maxSpeed * 0.5f);
                velocity.y = sin(angle) * (maxSpeed * 0.5f);
                changeDirectionTimer = 1.5f;
            }
        }
    }
    else if (nearestFood != nullptr) {
        state = State::FORAGING;
        float dx = nearestFood->getX() - x;
        float dy = nearestFood->getY() - y;
        float dist = std::hypot(dx, dy);

        if (dist < 12.0f) {
            velocity = {0, 0};
            nearestFood->setEnergy(nearestFood->getEnergy() - 15.0f * dt);
            energy = std::min(120.0f, energy + 8.0f * dt); // MAX_ENERGY
        } else {
            velocity.x = (dx / dist) * maxSpeed;
            velocity.y = (dy / dist) * maxSpeed;
            if (nearbyHerdCount > 0) {
                herdCenter.x /= nearbyHerdCount;
                herdCenter.y /= nearbyHerdCount;
                float hdx = herdCenter.x - x;
                float hdy = herdCenter.y - y;
                float hLen = std::hypot(hdx, hdy);
                if (hLen > 0) {
                    velocity.x = velocity.x * 0.7f + (hdx/hLen) * maxSpeed * 0.3f;
                    velocity.y = velocity.y * 0.7f + (hdy/hLen) * maxSpeed * 0.3f;
                }
            }
        }
    }
    else {
        state = State::WANDERING;
        if (nearbyHerdCount > 0) {
            herdCenter.x /= nearbyHerdCount;
            herdCenter.y /= nearbyHerdCount;
            float dx = herdCenter.x - x;
            float dy = herdCenter.y - y;
            float dist = std::hypot(dx, dy);
            if (dist > 15.0f) {
                velocity.x = (dx / dist) * (maxSpeed * 0.4f);
                velocity.y = (dy / dist) * (maxSpeed * 0.4f);
            } else { velocity = {0, 0}; }
        } else {
            changeDirectionTimer -= dt;
            if (changeDirectionTimer <= 0.0f) {
                float angle = (rand() % 360) * DEG2RAD;
                velocity.x = cos(angle) * (maxSpeed * 0.4f);
                velocity.y = sin(angle) * (maxSpeed * 0.4f);
                changeDirectionTimer = 2.0f;
            }
        }
    }

    // 3. WALL REPULSION (HARD OVERRIDE)
    float margin = 50.0f;
    float escapeSpeed = maxSpeed * 0.9f;
    if (x < margin) velocity.x = std::max(velocity.x, escapeSpeed);
    if (x > WORLD_WIDTH - margin) velocity.x = std::min(velocity.x, -escapeSpeed);
    if (y < margin) velocity.y = std::max(velocity.y, escapeSpeed);
    if (y > WORLD_HEIGHT - margin) velocity.y = std::min(velocity.y, -escapeSpeed);

    // 4. APPLY PHYSICS & METABOLISM
    velocity.x += acceleration.x * dt;
    velocity.y += acceleration.y * dt;
    acceleration = {0, 0};

    float maxVel = (state == State::FLEEING) ? maxSpeed * 1.2f : maxSpeed;
    float velLen = std::hypot(velocity.x, velocity.y);
    if (velLen > maxVel) {
        velocity.x = (velocity.x / velLen) * maxVel;
        velocity.y = (velocity.y / velLen) * maxVel;
    }

    x += velocity.x * dt;
    y += velocity.y * dt;
    x = std::max(0.0f, std::min((float)WORLD_WIDTH, x));
    y = std::max(0.0f, std::min((float)WORLD_HEIGHT, y));

    if (state == State::FLEEING) energy -= 6.0f * dt;
    else if (state == State::WANDERING || state == State::REPRODUCING) energy -= 1.5f * dt;
    else energy -= 2.5f * dt;
}

void Herbivore::draw() const {
    Color c = BLUE;
    if (state == State::FLEEING) c = SKYBLUE;
    else if (state == State::FORAGING) c = YELLOW;
    else if (state == State::REPRODUCING) c = PURPLE;
    else if (state == State::WANDERING) c = DARKBLUE;
    DrawCircle(x, y, 4.5f, c);
}