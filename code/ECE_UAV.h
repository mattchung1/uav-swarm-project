/*
Author: Aaron Huang, Dulani Wijayarathne, Matt Chung
Class: ECE6122
Last Date Modified: November 29, 2025

Description:
ECE_UAV class implementation for UAV simulation, including physics updates and control.

*/

#pragma once
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include "Vec3.h"
#include "PhysicsGlobals.h"
#include "PIDController.h"

// Flight state enumeration for state machine
enum class FlightState 
{
    IDLE,       // 0-5 seconds: Remain on ground
    ASCENT,     // Launch phase: Fly to (0,0,50) with max velocity 2 m/s
    ORBIT,      // Orbit phase: Fly on sphere surface for 60 seconds
    RETURN,     // Descend and travel back to launch position
    FINISHED    // Simulation complete
};

class ECE_UAV 
{
    private:
        // Declare kinematic variables
        Vec3 position;
        Vec3 velocity;
        Vec3 acceleration;

    // Home pad for return-to-start requirement
    Vec3 homePosition;
        
        // Mass of UAV (1 kg)
        double mass; // kg

        // Max force the UAV can exert
        double maxForce = 20.0; // Newtons

        // Gravity compensation force
        double gravityCompensation;

        // Control parameters
        Vec3 controlForce;

        // Thread for kinematic updates
        std::thread uavThread;
        
        // Flag to control thread execution
        std::atomic<bool> running;

        // Mutex for thread-safe access to kinematic data
        mutable std::mutex dataMutex;
        
        // ===== PERSON 3: FLIGHT CONTROL STATE MACHINE =====
        // Current flight state
        FlightState currentState;
        
        // Timing variables
        std::chrono::steady_clock::time_point startTime;
        std::chrono::steady_clock::time_point orbitStartTime;
        
        // PID controllers for sphere orbit (one per axis)
        PIDController pidX;
        PIDController pidY;
        PIDController pidZ;
        
        // Target point for ascent phase
        Vec3 targetPoint;
        
        // Sphere center and radius for orbit phase
        Vec3 sphereCenter;
        double sphereRadius;
        
        // Random velocity direction for orbit
        Vec3 randomDirection;
        
        // Color oscillation (ECE6122 requirement)
        double colorPhase;

        // Flag to indicate completion of the 60-second orbit window
        bool orbitCompleted;

    public:
        /*
        **************************
        CONSTRUCTOR / DESTRUCTOR & THREAD CONTROL
        **************************
        */
        //Declare member functions
        ECE_UAV(Vec3 initial_pos);
        ~ECE_UAV();

        // Start the thread running threadFunction
        void start();
        
        // Stop the thread
        void stop();

        // Check if thread is running
        bool isRunning() const { return running; }
        
        /*
        **************************
        GETTER FUNCTIONS
        **************************
        */
        // Get position of UAV (thread-safe)
        Vec3 getPosition();
        
        // Get velocity of UAV (thread-safe)
        Vec3 getVelocity();
        
        // Get acceleration of UAV (thread-safe)
        Vec3 getAcceleration();

        /*
        **************************
        PHYSICS UPDATE FUNCTIONS
        **************************
        */
        // Update kinematics (called by threadFunction)
        void updateKinematics(const Vec3& controlForce, double deltaTime);
        
        // Friend function declaration
        friend void threadFunction(ECE_UAV* pUAV);

        friend void checkCollisionsFor(ECE_UAV* me);

        /*
        **************************
        CONTROL FUNCTIONS
        **************************
        */
        // Calculate control forces for PID
        Vec3 calculateControlForces(const Vec3& target, double dt);

        // Reset controllers
        void resetControllers();
        
        // ===== PERSON 3: STATE MACHINE CONTROL =====
        /*
        Get current flight state
        Output: Current FlightState enum value
        */
        FlightState getFlightState();
        
        /*
        Get elapsed time since simulation start
        Output: Time in seconds
        */
        double getElapsedTime();
        
        /*
        Get color oscillation value for ECE6122 requirement
        Output: Value between 0.5 and 1.0 for color intensity
        */
        double getColorIntensity();

        // Whether this UAV has satisfied the 60-second orbit requirement
        bool hasCompletedOrbit();
        
        /*
        Calculate control force based on current flight state
        Input: deltaTime - time step for PID calculations
        Output: Force vector to apply to UAV
        */
        Vec3 calculateStateBasedForce(double deltaTime);
        
        /*
        Generate random direction for orbit phase
        */
        void generateRandomDirection();

};

// External thread function
void threadFunction(ECE_UAV* pUAV);
