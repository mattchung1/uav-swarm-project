/*
CPP file for ECE_UAV class. Defines functions used for the ECE_UAV class


*/

#include <iostream>
#include <chrono>
#include <thread>
#include "ECE_UAV.h"

// Constructor Function for ECE_UAV including position
ECE_UAV::ECE_UAV(double startX, double startY, double startZ)
    : mass(1.0),  // 1 kg mass
      x(startX), y(startY), z(startZ),
      vx(0.0), vy(0.0), vz(0.0),
      ax(0.0), ay(0.0), az(0.0),
      running(false)
{
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

// Get position of UAV (thread-safe)
void ECE_UAV::getPosition(double& outX, double& outY, double& outZ)
{
    std::lock_guard<std::mutex> lock(dataMutex);
    outX = x;
    outY = y;
    outZ = z;
}

// Get velocity of UAV (thread-safe)
void ECE_UAV::getVelocity(double& outVx, double& outVy, double& outVz)
{
    std::lock_guard<std::mutex> lock(dataMutex);
    outVx = vx;
    outVy = vy;
    outVz = vz;
}

// Get acceleration of UAV (thread-safe)
void ECE_UAV::getAcceleration(double& outAx, double& outAy, double& outAz)
{
    std::lock_guard<std::mutex> lock(dataMutex);
    outAx = ax;
    outAy = ay;
    outAz = az;
}

// Update kinematics using basic physics
void ECE_UAV::updateKinematics(double deltaTime)
{
    std::lock_guard<std::mutex> lock(dataMutex);
    
    // Update velocity: v = v0 + a*dt
    vx += ax * deltaTime;
    vy += ay * deltaTime;
    vz += az * deltaTime;
    
    // Update position: x = x0 + v*dt
    x += vx * deltaTime;
    y += vy * deltaTime;
    z += vz * deltaTime;
}

// Set acceleration (for external forces)
void ECE_UAV::setAcceleration(double newAx, double newAy, double newAz)
{
    std::lock_guard<std::mutex> lock(dataMutex);
    ax = newAx;
    ay = newAy;
    az = newAz;
}

// External thread function that updates kinematics every 10 msec
void threadFunction(ECE_UAV* pUAV)
{
    const double updateInterval = 0.01; // 10 msec in seconds
    
    while (pUAV->isRunning())
    {
        // Update the UAV's kinematic information
        pUAV->updateKinematics(updateInterval);
        
        // Sleep for 10 milliseconds
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

