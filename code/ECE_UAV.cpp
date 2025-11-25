/*
CPP file for ECE_UAV class. Defines functions used for the ECE_UAV class


*/

#include <iostream>
#include <chrono>
#include <thread>
#include "ECE_UAV.h"

/*
**************************
CONSTRUCTOR / DESTRUCTOR & THREAD CONTROL
**************************
*/
// Constructor Function for ECE_UAV including position
ECE_UAV::ECE_UAV(Vec3 initialPos)
    : position(initialPos), velocity(0, 0, 0), acceleration(0, 0, 0),
          mass(mass), maxForce(maxForce)
{
    this -> mass = 1.0; // kg
    this -> gravityCompensation = 10.0 * mass; // Newtons

    running = false;
    // PID controllers and other initializations can be added here


}

// Destructor function
ECE_UAV::~ECE_UAV()
{
    stop();
}

// Start the thread running threadFunction
void ECE_UAV::start()
{
    if (!running)
    {
        running = true;
        uavThread = std::thread(threadFunction, this);
    }
}

// Stop the thread
void ECE_UAV::stop()
{
    if (running)
    {
        running = false;
        if (uavThread.joinable())
        {
            uavThread.join();
        }
    }
}

// External thread function that updates kinematics every 10 msec
void threadFunction(ECE_UAV* pUAV)
{
    // Print statement for debugging
    std::cout << "Thread started for UAV!" << std::endl;

    const double updateInterval = 0.01; // 10 msec in seconds

    // ------ PERSON 3 WORKS HERE FOR PID
    // PLACEHOLDER FOR CONTROL FORCE, can be updated externally
    // can delete both of these Vec3 functions as they were only for testing
    
    Vec3 controlForce = Vec3(0, 0, 10); 
    // Vec3 target(0, 0, 20.0);

    
    while (pUAV->isRunning())
    {
        // ------ PERSON 3 WORKS HERE FOR PID
        // control logic only for testing
        // ------ Simple control logic to move towards target
        // Vec3 currentPos = pUAV->getPosition();
        // Vec3 direction = target - currentPos;
        // Vec3 controlForce = direction.normalized() * 20.0;
        // ----- end of simple control logic -----

        // ------ PERSON 2 WORKS HERE 
        // Update the UAV's kinematic information
        pUAV->updateKinematics(controlForce, updateInterval);

        checkCollisionsFor(pUAV);
        
        // Sleep for 10 milliseconds
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        // For testing: check for explosion condition
        Vec3 pos = pUAV->getPosition();
        if (pos.z > 100 || pos.z < -100) { 
             std::cout << "EXPLOSION DETECTED! Z: " << pos.z << std::endl;
        }
    }
}

/*
**************************
GETTER FUNCTIONS
**************************
*/
// Get position of UAV (thread-safe)
Vec3 ECE_UAV::getPosition()
{
    std::lock_guard<std::mutex> lock(dataMutex);
    return position;
}

// Get velocity of UAV (thread-safe)
Vec3 ECE_UAV::getVelocity()
{
    std::lock_guard<std::mutex> lock(dataMutex);
    return velocity;
}

// Get acceleration of UAV (thread-safe)
Vec3 ECE_UAV::getAcceleration()
{
    std::lock_guard<std::mutex> lock(dataMutex);
    return acceleration;
}

/*
**************************
PHYSICS UPDATE FUNCTIONS
**************************
*/
// Update kinematics using basic physics
void ECE_UAV::updateKinematics(const Vec3& controlForce, double deltaTime)
{
    std::lock_guard<std::mutex> lock(dataMutex);
    
    // Calculate total force
    Vec3 gravity(0, 0, -gravityCompensation);
    Vec3 total_force = controlForce + gravity;
    
    // Newton's second Lab F = ma
    acceleration = total_force / mass;
    
    // Update position components x = x0 + v0*t + 0.5*a*t^2
    position += velocity * deltaTime + acceleration * (0.5 * deltaTime * deltaTime);

    // Update velocity components v = v0 + a*t
    velocity += acceleration * deltaTime;

    // Optional: Add ground constraint
    if (position.z < 0) {
        position.z = 0;
        if (velocity.z < 0) velocity.z = 0;
    }
}

