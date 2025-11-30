/*
CPP file for ECE_UAV class. Defines functions used for the ECE_UAV class


*/

#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>
#include <random>
#include "ECE_UAV.h"

/*
**************************
CONSTRUCTOR / DESTRUCTOR & THREAD CONTROL
**************************
*/
// Constructor Function for ECE_UAV including position
ECE_UAV::ECE_UAV(Vec3 initialPos)
    : position(initialPos), velocity(0, 0, 0), acceleration(0, 0, 0),
          mass(mass), maxForce(maxForce),
          currentState(FlightState::IDLE),
          targetPoint(0, 0, 50),
          sphereCenter(0, 0, 50),
          sphereRadius(10.0),
          colorPhase(0.0),
          pidX(50.0, 0.5, 10.0),  // Tuned PID gains for X axis
          pidY(50.0, 0.5, 10.0),  // Tuned PID gains for Y axis
          pidZ(50.0, 0.5, 10.0)   // Tuned PID gains for Z axis
{
    this -> mass = 1.0; // kg
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

        // For debugging: check for explosion condition
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
        // Calculate direction to target point (0, 0, 50)
        Vec3 directionToTarget = targetPoint - position;
        double distanceToTarget = directionToTarget.magnitude();
        
        // Check if we've reached the target (within 10m of sphere center)
        if (distanceToTarget <= sphereRadius)
        {
            // Transition to ORBIT
            currentState = FlightState::ORBIT;
            orbitStartTime = std::chrono::steady_clock::now();
            generateRandomDirection();
            pidX.reset();
            pidY.reset();
            pidZ.reset();
            std::cout << "UAV transitioning to ORBIT state" << std::endl;
        }
        else
        {
            // Calculate force to move towards target
            Vec3 desiredDirection = directionToTarget.normalized();
            
            // Limit maximum velocity to 2 m/s during ascent
            double currentSpeed = velocity.magnitude();
            const double maxAscentSpeed = 2.0;
            
            if (currentSpeed < maxAscentSpeed)
            {
                // Apply force in direction of target
                force = desiredDirection * maxForce;
            }
            else
            {
                // Apply braking force if moving too fast
                Vec3 velocityDirection = velocity.normalized();
                force = velocityDirection * (-maxForce * 0.5);
                
                // Still need to counter gravity
                force.z += gravityCompensation;
            }
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
            currentState = FlightState::FINISHED;
            std::cout << "UAV finished orbit - simulation complete" << std::endl;
            force = Vec3(0, 0, gravityCompensation); // Hover in place
        }
        else
        {
            // Use PID controller to maintain position on sphere surface
            
            // Calculate current distance from sphere center
            Vec3 vectorFromCenter = position - sphereCenter;
            double currentRadius = vectorFromCenter.magnitude();
            
            // Error: difference between desired radius and actual radius
            double radialError = sphereRadius - currentRadius;
            
            // PID control for radial distance
            // Apply force towards/away from center to maintain sphere radius
            Vec3 radialDirection = vectorFromCenter.normalized();
            double radialForce = pidX.calculate(sphereRadius, currentRadius, deltaTime);
            
            // Apply radial correction force
            Vec3 radialCorrectionForce = radialDirection * radialForce;
            
            // Add tangential force for random movement along sphere surface
            // Project random direction onto tangent plane of sphere
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
            if (directionChangeCounter > 100) // Change every ~1 second
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
