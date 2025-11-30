/*
Physics Globals for Collision detection
*/


#include "PhysicsGlobals.h"
#include "ECE_UAV.h"
#include <iostream>
#include <atomic>
#include <algorithm>

// Define the global UAV list pointer
std::vector<ECE_UAV*>* GLOBAL_UAV_LIST = nullptr;

const double collisionThreshold = 0.01; // 1 centimeter clearance
static std::atomic<double> gBoundingRadius(0.1); // Default 10 cm radius (20 cm cube)

void setUAVBoundingRadius(double radius)
{
    double safeRadius = std::max(radius, 0.01);
    gBoundingRadius.store(safeRadius);
}


void checkCollisionsFor(ECE_UAV* me)
{
    if (GLOBAL_UAV_LIST == nullptr)
    {
        return; // No UAVs to check against
    }

    Vec3 myPos = me-> getPosition();

    for (ECE_UAV* other : *GLOBAL_UAV_LIST) 
    {
        if (me == other) continue; // skip self collision check

        Vec3 otherPos = other-> getPosition();
        double distance = myPos.distance(otherPos);
        double boundingRadius = gBoundingRadius.load();
        const double triggerDistance = (2.0 * boundingRadius) + collisionThreshold;
        if (distance < triggerDistance)
        {
            // Swap velocities
            {
                // Lock both UAVs' data mutexes
                std::unique_lock<std::mutex> uavLock1(me->dataMutex, std::defer_lock);
                std::unique_lock<std::mutex> uavLock2(other->dataMutex, std::defer_lock);

                // Lock both UAVs' mutexes without deadlock
                std::lock(uavLock1, uavLock2);

                // Update positions inside lock to avoid stale data
                myPos = me->position;
                otherPos = other->position;

                // Swap velocities
                Vec3 dummyVel = me->velocity;
                me->velocity = other->velocity;
                other->velocity = dummyVel;

                // Gently separate overlapping UAVs to reduce repeat collisions
                Vec3 separation = myPos - otherPos;
                double separationMag = separation.magnitude();
                Vec3 correctionDir = separationMag > 1e-6 ? separation / separationMag : Vec3(1, 0, 0);
                double overlap = triggerDistance - separationMag;
                if (overlap < 0.0)
                {
                    overlap = 0.0;
                }
                Vec3 correction = correctionDir * (overlap * 0.5);
                me->position += correction;
                other->position = other->position - correction;

                // Optional: Print collision event (once per detection)
                std::cout << "Collision detected between UAVs at positions: ("
                            << myPos.x << ", " << myPos.y << ", " << myPos.z << ") and ("
                            << otherPos.x << ", " << otherPos.y << ", " << otherPos.z << ")\n";
            }
        }
    }
}