#include "Carnivore.h"
#include "../core/Quadtree.h"
#include "../world/Grid.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>

Carnivore::Carnivore(float x, float y)
    : Organism(x, y, OrganismType::Carnivore),
      changeDirectionTimer(0.0f),
      stalkTimer(0.0f),
      burstTimer(0.0f),
      isBursting(false),
      chaseRadius(100.0f),  // NEW: Actively chase if within 100px
      catchRadius(25.0f) {
    maxSpeed = 100.0f;
    maxForce = 250.0f;
    visionRadius = 150.0f;
    energy = 150.0f; // Your new starting value
    velocity = {0, 0};
    acceleration = {0, 0};
    state = State::RELOCATING;
}

void Carnivore::update(float dt, Grid& grid, const Quadtree& tree, std::vector<Organism*>& newEntities) {
    // Leave danger trail (less visible when stalking)
    float dangerAmount = (state == State::STALKING || state == State::RELOCATING) ? 0.2f : 0.6f;
    grid.addDanger(x, y, dangerAmount, isBursting ? 30.0f : 20.0f);

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
            if (dist < minPreyDist) {
                minPreyDist = dist;
                targetPrey = other;
            }
        } else if (other->getType() == OrganismType::Carnivore && other != this) {
            if (dist < minMateDist) {
                minMateDist = dist;
                mateTarget = other;
            }
        }
    }

    // === BEHAVIOR PRIORITY SYSTEM ===

    if (energy < CRITICAL_ENERGY) {
        // === PRIORITY 1: DESPERATE HUNT ===
        state = State::DESPERATE_HUNT;
        isBursting = true;
        burstTimer = 3.0f;

        if (targetPrey != nullptr && minPreyDist < visionRadius) {
            float dx = targetPrey->getX() - x;
            float dy = targetPrey->getY() - y;
            float len = std::hypot(dx, dy);
            velocity.x = (dx / len) * (maxSpeed * 1.8f);
            velocity.y = (dy / len) * (maxSpeed * 1.8f);

            if (minPreyDist < catchRadius) {
                energy = std::min(MAX_ENERGY, energy + 60.0f * dt);
                targetPrey->setEnergy(targetPrey->getEnergy() - 100.0f * dt);
            }
        } else {
            // Sprint randomly to find prey
            changeDirectionTimer -= dt;
            if (changeDirectionTimer <= 0.0f) {
                float angle = (rand() % 360) * DEG2RAD;
                velocity.x = cos(angle) * (maxSpeed * 1.5f);
                velocity.y = sin(angle) * (maxSpeed * 1.5f);
                changeDirectionTimer = 0.5f;
            }
        }
    }
    else if (energy >= REPRODUCE_ENERGY) {
        // === PRIORITY 2: REPRODUCE ===
        state = State::REPRODUCING;
        isBursting = false;

        if (mateTarget != nullptr && minMateDist < 20.0f) {
            velocity = {0, 0};
            energy -= 90.0f; // High cost to reproduce
            float offsetX = (rand() % 30) - 15;
            float offsetY = (rand() % 30) - 15;
            newEntities.push_back(new Carnivore(x + offsetX, y + offsetY));
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
                changeDirectionTimer = 2.0f;
            }
        }
    }
    else if (targetPrey != nullptr && minPreyDist < chaseRadius) {
        // === PRIORITY 3: ACTIVE CHASE / HUNT (THE FIX) ===
        state = State::HUNTING;
        isBursting = (minPreyDist < 40.0f); // Only burst when very close

        float dx = targetPrey->getX() - x;
        float dy = targetPrey->getY() - y;
        float len = std::hypot(dx, dy);

        float speedMultiplier = isBursting ? 1.6f : 1.2f;
        velocity.x = (dx / len) * (maxSpeed * speedMultiplier);
        velocity.y = (dy / len) * (maxSpeed * speedMultiplier);

        if (minPreyDist < catchRadius) {
            energy = std::min(MAX_ENERGY, energy + 60.0f * dt);
            targetPrey->setEnergy(targetPrey->getEnergy() - 100.0f * dt);
        }
    }
    else if (targetPrey != nullptr && minPreyDist < visionRadius) {
        // === PRIORITY 4: STALKING (Prey is visible but far) ===
        state = State::STALKING;
        isBursting = false;

        float dx = targetPrey->getX() - x;
        float dy = targetPrey->getY() - y;
        float len = std::hypot(dx, dy);

        // Slow, stealthy approach
        velocity.x = (dx / len) * (maxSpeed * 0.4f);
        velocity.y = (dy / len) * (maxSpeed * 0.4f);
    }
    else {
        // === PRIORITY 5: RELOCATE / WANDER (Conserve energy) ===
        state = State::RELOCATING;
        isBursting = false;

        changeDirectionTimer -= dt;
        if (changeDirectionTimer <= 0.0f) {
            float angle = (rand() % 360) * DEG2RAD;
            velocity.x = cos(angle) * (maxSpeed * 0.3f);
            velocity.y = sin(angle) * (maxSpeed * 0.3f);
            changeDirectionTimer = 3.0f + (rand() % 200) / 100.0f;
        }
    }

    // Handle burst timer expiration
    if (isBursting) {
        burstTimer -= dt;
        if (burstTimer <= 0.0f) {
            isBursting = false;
        }
    }

    // Apply physics
    velocity.x += acceleration.x * dt;
    velocity.y += acceleration.y * dt;
    acceleration = {0, 0};

    // Clamp velocity
    float maxVel = isBursting ? maxSpeed * 1.8f : maxSpeed;
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

    // === METABOLISM ===
    if (state == State::STALKING || state == State::RELOCATING) {
        energy -= 1.5f * dt;  // Very slow drain when conserving
    } else if (isBursting) {
        energy -= (8.0f + BURST_COST) * dt;  // Massive drain when bursting
    } else {
        energy -= 3.0f * dt;  // Normal drain when chasing
    }
}

void Carnivore::draw() const {
    Color c = RED;
    float radius = 5.0f;

    if (state == State::DESPERATE_HUNT) {
        c = ORANGE;
        radius = 6.5f;
    } else if (state == State::HUNTING) {
        c = MAGENTA;
        radius = 5.5f;
    } else if (state == State::STALKING) {
        c = MAROON;
        radius = 4.0f; // Smaller when hiding
    } else if (state == State::REPRODUCING) {
        c = PINK;
    } else if (state == State::RELOCATING) {
        c = Fade(RED, 0.5f); // Semi-transparent when conserving
    }

    DrawCircle(x, y, radius, c);

    // Draw chase radius subtly when hunting
    if (state == State::HUNTING || state == State::DESPERATE_HUNT) {
        DrawCircleLines(x, y, chaseRadius, Fade(RED, 0.15f));
    }
}