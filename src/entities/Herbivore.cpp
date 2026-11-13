#include "Herbivore.h"
#include "../core/Quadtree.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>

Herbivore::Herbivore(float x, float y)
    : Organism(x, y, OrganismType::Herbivore),
      changeDirectionTimer(0.0f),
      herdCenter{0, 0},
      nearbyHerdCount(0) {
    maxSpeed = 75.0f;       // Slightly faster to escape and reach food
    maxForce = 200.0f;      // Sharp turning ability
    visionRadius = 150.0f;  // Increased vision to spot food and mates easier
    energy = 50.0f;        // Start with a decent buffer
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
            if (dist < minThreatDist) {
                minThreatDist = dist;
                nearestThreat = other;
            }
        } else if (other->getType() == OrganismType::Autotroph) {
            if (dist < minFoodDist) {
                minFoodDist = dist;
                nearestFood = other;
            }
        } else if (other->getType() == OrganismType::Herbivore) {
            herdCenter.x += other->getX();
            herdCenter.y += other->getY();
            nearbyHerdCount++;

            if (dist < minMateDist) {
                minMateDist = dist;
                nearestMate = other;
            }
        }
    }

    // 2. BEHAVIOR PRIORITY SYSTEM

    // PRIORITY 1: SURVIVAL (Flee from predators within 80px)
    if (nearestThreat != nullptr && minThreatDist < 80.0f) {
        state = State::FLEEING;
        float dx = x - nearestThreat->getX();
        float dy = y - nearestThreat->getY();
        float len = std::hypot(dx, dy);

        if (len > 0) {
            velocity.x = (dx / len) * (maxSpeed * 1.6f); // Sprint away
            velocity.y = (dy / len) * (maxSpeed * 1.6f);
        }
    }
    // PRIORITY 2: REPRODUCE (If energy is high enough)
    else if (energy >= REPRODUCE_ENERGY) {
        state = State::REPRODUCING;

        // Try to stay near the herd to reproduce safely
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
                velocity = {0, 0}; // Stop in the safety of the herd

                // SPAWN BABY if we are near at least one other herbivore
                if (nearestMate != nullptr && minMateDist < 30.0f) {
                    energy -= 50.0f; // Cost of reproduction
                    float offsetX = (rand() % 20) - 10;
                    float offsetY = (rand() % 20) - 10;
                    newEntities.push_back(new Herbivore(x + offsetX, y + offsetY));
                }
            }
        } else {
            // No herd nearby? Wander to find one before reproducing
            changeDirectionTimer -= dt;
            if (changeDirectionTimer <= 0.0f) {
                float angle = (rand() % 360) * DEG2RAD;
                velocity.x = cos(angle) * (maxSpeed * 0.5f);
                velocity.y = sin(angle) * (maxSpeed * 0.5f);
                changeDirectionTimer = 1.5f;
            }
        }
    }
    // PRIORITY 3: FORAGE (Always eat if hungry and no immediate threat)
    else if (nearestFood != nullptr) {
        state = State::FORAGING;
        float dx = nearestFood->getX() - x;
        float dy = nearestFood->getY() - y;
        float dist = std::hypot(dx, dy);

        if (dist < 12.0f) {
            // EAT! Stop moving and drain the plant
            velocity = {0, 0};
            nearestFood->setEnergy(nearestFood->getEnergy() - 15.0f * dt); // Plants die faster
            energy = std::min(MAX_ENERGY, energy + 8.0f * dt);            // Herbivores gain energy faster
        } else {
            // Move toward food
            velocity.x = (dx / dist) * maxSpeed;
            velocity.y = (dy / dist) * maxSpeed;

            // Blend with herd cohesion (30% weight) so they move as a group toward food
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
    // PRIORITY 4: HERD / WANDER (Default state when safe and no food in sight)
    else {
        state = State::WANDERING;

        if (nearbyHerdCount > 0) {
            // Stick with the herd for safety
            herdCenter.x /= nearbyHerdCount;
            herdCenter.y /= nearbyHerdCount;
            float dx = herdCenter.x - x;
            float dy = herdCenter.y - y;
            float dist = std::hypot(dx, dy);

            if (dist > 15.0f) {
                velocity.x = (dx / dist) * (maxSpeed * 0.4f);
                velocity.y = (dy / dist) * (maxSpeed * 0.4f);
            } else {
                velocity = {0, 0}; // Rest with the herd
            }
        } else {
            // Wander randomly to find food/herd
            changeDirectionTimer -= dt;
            if (changeDirectionTimer <= 0.0f) {
                float angle = (rand() % 360) * DEG2RAD;
                velocity.x = cos(angle) * (maxSpeed * 0.4f);
                velocity.y = sin(angle) * (maxSpeed * 0.4f);
                changeDirectionTimer = 2.0f;
            }
        }
    }

    // 3. APPLY PHYSICS & METABOLISM
    velocity.x += acceleration.x * dt;
    velocity.y += acceleration.y * dt;
    acceleration = {0, 0}; // Reset acceleration

    // Clamp velocity
    float maxVel = (state == State::FLEEING) ? maxSpeed * 1.6f : maxSpeed;
    float velLen = std::hypot(velocity.x, velocity.y);
    if (velLen > maxVel) {
        velocity.x = (velocity.x / velLen) * maxVel;
        velocity.y = (velocity.y / velLen) * maxVel;
    }

    // Update position
    x += velocity.x * dt;
    y += velocity.y * dt;

    // Screen bounds (bounce)
    if (x < 0 || x > WORLD_WIDTH) { velocity.x *= -1; x = std::max(0.0f, std::min((float)WORLD_WIDTH, x)); }
    if (y < 0 || y > WORLD_HEIGHT) { velocity.y *= -1; y = std::max(0.0f, std::min((float)WORLD_HEIGHT, y)); }

    // Metabolism
    if (state == State::FLEEING) {
        energy -= 8.0f * dt;  // High cost for sprinting
    } else if (state == State::WANDERING || state == State::REPRODUCING) {
        energy -= 1.5f * dt;  // Low cost for resting/moving slowly
    } else {
        energy -= 3.0f * dt;  // Normal cost
    }
}

void Herbivore::draw() const {
    Color c = BLUE;
    if (state == State::FLEEING) c = SKYBLUE;
    else if (state == State::FORAGING) c = YELLOW;
    else if (state == State::REPRODUCING) c = PURPLE;
    else if (state == State::WANDERING) c = DARKBLUE;

    DrawCircle(x, y, 4.5f, c);
}