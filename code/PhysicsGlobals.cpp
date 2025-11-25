/*
Physics Globals for Collision detection
*/


#include "PhysicsGlobals.h"
#include "ECE_UAV.h"
#include <iostream>

// Define the global UAV list pointer
std::vector<ECE_UAV*>* GLOBAL_UAV_LIST = nullptr;

const double collisionThreshold = 0.01; // 1 centimeter
const double boundingRadius = 0.1; // 10 cm UAV bounding box


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
        
        if (distance < (boundingRadius + collisionThreshold + collisionThreshold))
        {
            // Swap velocities
            {
                // Lock both UAVs' data mutexes
                std::unique_lock<std::mutex> uavLock1(me->dataMutex, std::defer_lock);
                std::unique_lock<std::mutex> uavLock2(other->dataMutex, std::defer_lock);

                // Lock both UAVs' mutexes without deadlock
                std::lock(uavLock1, uavLock2);

                // Swap velocities
                Vec3 dummyVel = me->velocity;
                me->velocity = other->velocity;
                other->velocity = dummyVel;

                // Optional: Print collision event
                std::cout << "Collision detected between UAVs at positions: ("
                            << myPos.x << ", " << myPos.y << ", " << myPos.z << ") and ("
                            << otherPos.x << ", " << otherPos.y << ", " << otherPos.z << ")\n";
            }
        }
    }
}