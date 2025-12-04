/*
Author: Aaron Huang, Dulani Wijayarathne, Matt Chung
Class: ECE6122
Last Date Modified: November 29, 2025

Description:
ECE_UAV class implementation for UAV simulation, including physics updates and control.

*/
#define _USE_MATH_DEFINES

#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <cmath>
#include <random>
#include "ECE_UAV.h"


// Helper to replace std::clamp in C++14
template <typename T>
const T& my_clamp(const T& v, const T& lo, const T& hi) {
    return (v < lo) ? lo : (hi < v ? hi : v);
}


/*
**************************
CONSTRUCTOR / DESTRUCTOR & THREAD CONTROL
**************************
*/
// Constructor Function for ECE_UAV including position
ECE_UAV::ECE_UAV(Vec3 initialPos)
    : position(initialPos), velocity(0, 0, 0), acceleration(0, 0, 0),
          mass(1.0), maxForce(20.0),  // FIXED: Initialize with actual values!
          currentState(FlightState::IDLE),
          targetPoint(0, 0, 50),
          sphereCenter(0, 0, 50),
          sphereRadius(10.0),
          colorPhase(0.0),
             orbitCompleted(false),
          pidX(8.0, 0.1, 3.0),  // Tuned PID gains for radial control
          pidY(8.0, 0.1, 3.0),  // Reserved for future axis control
          pidZ(8.0, 0.1, 3.0)   // Reserved for future axis control
{
    // Mass and maxForce already initialized in member initializer list
    this -> gravityCompensation = 10.0 * mass; // Newtons

    running = false;
    
    // Initialize timing
    startTime = std::chrono::steady_clock::now();
    
    // Initialize random direction for orbit
    randomDirection = Vec3(1, 0, 0);
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

    // Main control loop
    while (pUAV->isRunning())
    {
        // ===== PERSON 3: CALCULATE STATE-BASED CONTROL FORCE =====
        Vec3 controlForce = pUAV->calculateStateBasedForce(updateInterval);

        // ===== PERSON 2: UPDATE PHYSICS =====
        // Update the UAV's kinematic information
        pUAV->updateKinematics(controlForce, updateInterval);

        // Check for collisions with other UAVs
        checkCollisionsFor(pUAV);
        
        // Sleep for 10 milliseconds
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        // Debug logging disabled: explosion detection was noisy in normal runs
        // Vec3 pos = pUAV->getPosition();
        // if (pos.z > 100 || pos.z < -100) {
        //      std::cout << "EXPLOSION DETECTED! Z: " << pos.z << std::endl;
        // }
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

/*
**************************
PERSON 3: STATE MACHINE AND CONTROL FUNCTIONS
**************************
*/

/*
Get current flight state (thread-safe)
*/
FlightState ECE_UAV::getFlightState()
{
    std::lock_guard<std::mutex> lock(dataMutex);
    return currentState;
}

/*
Get elapsed time since simulation start
*/
double ECE_UAV::getElapsedTime()
{
    auto currentTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = currentTime - startTime;
    return elapsed.count();
}

/*
Get color intensity for ECE6122 requirement
Oscillates between 0.5 and 1.0 at 0.5 Hz frequency
*/
double ECE_UAV::getColorIntensity()
{
    std::lock_guard<std::mutex> lock(dataMutex);
    // Calculate intensity: 0.75 + 0.25 * sin(2*pi*0.5*t)
    // This oscillates between 0.5 (half) and 1.0 (full)
    double intensity = 0.75 + 0.25 * std::sin(colorPhase);
    return intensity;
}

bool ECE_UAV::hasCompletedOrbit()
{
    std::lock_guard<std::mutex> lock(dataMutex);
    return orbitCompleted;
}

/*
Generate random direction for orbit phase
Creates a random tangent vector on the sphere surface
*/
void ECE_UAV::generateRandomDirection()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(-1.0, 1.0);
    
    // Generate random direction
    randomDirection = Vec3(dis(gen), dis(gen), dis(gen));
    randomDirection = randomDirection.normalized();
}

/*
Calculate control force based on current flight state
This is the main control logic for Person 3
*/
Vec3 ECE_UAV::calculateStateBasedForce(double deltaTime)
{
    std::lock_guard<std::mutex> lock(dataMutex);
    
    double elapsedTime = getElapsedTime();
    Vec3 force(0, 0, 0);
    
    // Update color phase for ECE6122 requirement and keep it bounded
    colorPhase += 2.0 * M_PI * 0.5 * deltaTime; // 0.5 Hz oscillation
    if (colorPhase > 2.0 * M_PI) {
        colorPhase = std::fmod(colorPhase, 2.0 * M_PI);
    }
    
    // ===== STATE A: IDLE (0-5 seconds) =====
    if (currentState == FlightState::IDLE)
    {
        if (elapsedTime < 5.0)
        {
            // Remain on ground - apply force to counter gravity
            force = Vec3(0, 0, gravityCompensation);
            
            // Ensure zero velocity while grounded
            velocity = Vec3(0, 0, 0);
        }
        else
        {
            // Transition to ASCENT after 5 seconds
            currentState = FlightState::ASCENT;
            std::cout << "UAV transitioning to ASCENT state" << std::endl;
        }
    }
    
    // ===== STATE B: ASCENT =====
    else if (currentState == FlightState::ASCENT)
    {
        // Calculate direction to sphere center (for distance checking)
        Vec3 directionToCenter = sphereCenter - position;
        double distanceToCenter = directionToCenter.magnitude();
        
        // Calculate how far we are from the sphere SURFACE
        double distanceFromSurface = distanceToCenter - sphereRadius;
        
    // Desired direction is straight toward the sphere center
    Vec3 desiredDirection = directionToCenter.normalized();
        
        // Debug output
        static int debugCounter = 0;
        if (debugCounter++ % 100 == 0) {
            std::cout << "ASCENT - Pos: (" << position.x << ", " << position.y << ", " << position.z << ")"
                      << " | Dist to center: " << distanceToCenter << "m"
                      << " | Dist from surface: " << distanceFromSurface << "m"
                      << " | Speed: " << velocity.magnitude() << " m/s"
                      << " | Vel: (" << velocity.x << ", " << velocity.y << ", " << velocity.z << ")" << std::endl;
        }
        
        // Check if we've reached the sphere surface (within 0.5m tolerance)
        if (distanceFromSurface <= 0.5)
        {
            // Transition to ORBIT
            currentState = FlightState::ORBIT;
            orbitStartTime = std::chrono::steady_clock::now();
            generateRandomDirection();
            pidX.reset();
            pidY.reset();
            pidZ.reset();
            std::cout << "UAV transitioning to ORBIT state. Distance to center: " << distanceToCenter 
                      << "m (radius: " << sphereRadius << "m)" << std::endl;
        }
        else
        {
            // Calculate direction toward sphere center (target point at (0,0,50))
            const double maxAscentSpeed = 2.0;

            // Speed component toward the target (positive when moving inward)
            double speedTowardTarget = velocity.x * desiredDirection.x +
                                       velocity.y * desiredDirection.y +
                                       velocity.z * desiredDirection.z;

            // Base thrust to hover plus directional control budget
            Vec3 thrust(0.0, 0.0, gravityCompensation);
            double availableForce = std::max(0.0, maxForce - gravityCompensation);

            // Taper the target speed as we approach the sphere to avoid overshoot
            double normalizedDistance = my_clamp(distanceFromSurface / sphereRadius, 0.0, 1.0);
            double targetSpeed = normalizedDistance * maxAscentSpeed;

            // Directional control: accelerate or brake along desiredDirection
            double speedError = targetSpeed - speedTowardTarget;
            double controlRatio = 0.0;
            if (maxAscentSpeed > 0.0)
            {
                controlRatio = my_clamp(speedError / maxAscentSpeed, -1.0, 1.0);
            }
            thrust += desiredDirection * (availableForce * controlRatio);

            // Lateral damping to keep approach aligned with the target direction
            Vec3 lateralVelocity = velocity - desiredDirection * speedTowardTarget;
            double lateralSpeed = lateralVelocity.magnitude();
            if (lateralSpeed > 0.05 && availableForce > 0.0)
            {
                Vec3 lateralDir = lateralVelocity.normalized();
                double lateralRatio = my_clamp(lateralSpeed / maxAscentSpeed, 0.0, 1.0);
                thrust += lateralDir * (-availableForce * 0.6 * lateralRatio);
            }

            // Cap the thrust vector so we never exceed vehicle capability
            double thrustMagnitude = thrust.magnitude();
            if (thrustMagnitude > maxForce && thrustMagnitude > 0.0)
            {
                thrust = thrust * (maxForce / thrustMagnitude);
            }

            force = thrust;
        }
    }
    
    // ===== STATE C: ORBIT ON SPHERE =====
    else if (currentState == FlightState::ORBIT)
    {
        // Check if orbit phase is complete (60 seconds)
        auto currentTime = std::chrono::steady_clock::now();
        std::chrono::duration<double> orbitElapsed = currentTime - orbitStartTime;
        
            if (orbitElapsed.count() >= 60.0)
            {
                orbitCompleted = true;
            }

            // Use PID controller to maintain position on sphere surface even after completion

            // Calculate current distance from sphere center
            Vec3 vectorFromCenter = position - sphereCenter;
            double currentRadius = vectorFromCenter.magnitude();
        
            // PID control for radial distance
            Vec3 radialDirection = vectorFromCenter.normalized();
            double radialForce = pidX.calculate(sphereRadius, currentRadius, deltaTime);
        
            // Apply radial correction force
            Vec3 radialCorrectionForce = radialDirection * radialForce;
        
            // Add tangential force for random movement along sphere surface
            Vec3 tangentDirection = randomDirection - radialDirection * 
                                   (randomDirection.x * radialDirection.x + 
                                    randomDirection.y * radialDirection.y + 
                                    randomDirection.z * radialDirection.z);
            tangentDirection = tangentDirection.normalized();
        
            // Maintain velocity between 2-10 m/s
            double currentSpeed = velocity.magnitude();
            const double minOrbitSpeed = 2.0;
            const double maxOrbitSpeed = 10.0;
        
            double tangentialForce = 0.0;
            if (currentSpeed < minOrbitSpeed)
            {
                tangentialForce = 5.0; // Accelerate
            }
            else if (currentSpeed > maxOrbitSpeed)
            {
                tangentialForce = -5.0; // Decelerate
            }
            else
            {
                tangentialForce = 2.0; // Maintain speed
            }
        
            Vec3 tangentForce = tangentDirection * tangentialForce;
        
            // Periodically change random direction
            static int directionChangeCounter = 0;
            directionChangeCounter++;
            if (directionChangeCounter > 200)
            {
                generateRandomDirection();
                directionChangeCounter = 0;
            }
        
            // Total force = radial correction + tangential movement + gravity compensation
            force = radialCorrectionForce + tangentForce;
            force.z += gravityCompensation;
        
            // Clamp force to maximum
            double forceMagnitude = force.magnitude();
            if (forceMagnitude > maxForce)
            {
                force = force.normalized() * maxForce;
            }
        {
            // Maintain orbit on 10 m sphere using radial/tangential control
            Vec3 vectorFromCenter = position - sphereCenter;
            double currentRadius = vectorFromCenter.magnitude();
            if (currentRadius < 1e-6)
            {
                vectorFromCenter = Vec3(0, 0, 1e-6);
                currentRadius = 1e-6;
            }

            Vec3 radialDirection = vectorFromCenter.normalized();
            double radialError = currentRadius - sphereRadius; // positive if outside the sphere
            double radialSpeed = velocity.x * radialDirection.x +
                                 velocity.y * radialDirection.y +
                                 velocity.z * radialDirection.z;

            double availableForce = std::max(0.0, maxForce - gravityCompensation);

            // PID on radial error with additional damping on radial velocity
            double radialControl = pidX.calculate(0.0, radialError, deltaTime) - 2.0 * radialSpeed;
            radialControl = my_clamp(radialControl, -availableForce, availableForce);
            Vec3 radialCorrectionForce = radialDirection * radialControl;

            // Tangential direction (project random vector onto tangent plane)
            Vec3 tangentSeed = randomDirection - radialDirection *
                               (randomDirection.x * radialDirection.x +
                                randomDirection.y * radialDirection.y +
                                randomDirection.z * radialDirection.z);
            if (tangentSeed.magnitude() < 1e-6)
            {
                generateRandomDirection();
                tangentSeed = randomDirection - radialDirection *
                              (randomDirection.x * radialDirection.x +
                               randomDirection.y * radialDirection.y +
                               randomDirection.z * radialDirection.z);
            }
            Vec3 tangentDirection = tangentSeed.normalized();

            Vec3 tangentialVelocity = velocity - radialDirection * radialSpeed;
            double tangentialSpeed = tangentialVelocity.magnitude();

            // Debug output for orbit speed
            static int orbitDebugCounter = 0;
            if (orbitDebugCounter++ % 100 == 0) {
                std::cout << "ORBIT - Tangential Speed: " << tangentialSpeed << " m/s"
                          << " | Radial Error: " << radialError << "m"
                          << " | Radius: " << currentRadius << "m" << std::endl;
            }

            const double minOrbitSpeed = 2.0;
            const double maxOrbitSpeed = 10.0;
            const double targetOrbitSpeed = 6.0;

            double tangentialRatio = 0.0;
            if (tangentialSpeed < minOrbitSpeed)
            {
                tangentialRatio = my_clamp((minOrbitSpeed - tangentialSpeed) / minOrbitSpeed, 0.0, 1.0);
            }
            else if (tangentialSpeed > maxOrbitSpeed)
            {
                tangentialRatio = -my_clamp((tangentialSpeed - maxOrbitSpeed) / maxOrbitSpeed, 0.0, 1.0);
            }
            else
            {
                double midBandError = (targetOrbitSpeed - tangentialSpeed) / targetOrbitSpeed;
                tangentialRatio = my_clamp(midBandError, -0.5, 0.5);
            }

            Vec3 tangentControlDirection = tangentDirection;
            if (tangentialRatio < 0.0 && tangentialSpeed > 0.05)
            {
                tangentControlDirection = tangentialVelocity.normalized();
            }

            Vec3 tangentialForce = tangentControlDirection * (availableForce * tangentialRatio);

            // Periodically refresh random tangent directions to keep paths varied
            static int directionChangeCounter = 0;
            directionChangeCounter++;
            if (directionChangeCounter > 200)
            {
                generateRandomDirection();
                directionChangeCounter = 0;
            }

            Vec3 totalForce = radialCorrectionForce + tangentialForce;
            totalForce.z += gravityCompensation;

            double totalMagnitude = totalForce.magnitude();
            if (totalMagnitude > maxForce && totalMagnitude > 0.0)
            {
                totalForce = totalForce * (maxForce / totalMagnitude);
            }

            force = totalForce;
        }
    }
    
    // ===== STATE D: FINISHED =====
    else if (currentState == FlightState::FINISHED)
    {
        // Hover in place
        force = Vec3(0, 0, gravityCompensation);
    }
    
    return force;
}
