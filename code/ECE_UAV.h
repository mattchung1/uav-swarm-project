/*
Header file for ECE_UAV class

*/


#pragma once
#include <thread>
#include <atomic>
#include <mutex>
#include "Vec3.h"

class ECE_UAV 
{
    private:
        // Declare kinematic variables
        Vec3 position;
        Vec3 velocity;
        Vec3 acceleration;
        
        // Mass of UAV (1 kg)
        double mass = 1.0; // kg

        // Max force the UAV can exert
        double maxForce = 20.0; // Newtons

        // Gravity compensation force
        double gravityCompensation = 10.0 * mass;

        // Control parameters
        Vec3 controlForce;

        // Thread for kinematic updates
        std::thread uavThread;
        
        // Flag to control thread execution
        std::atomic<bool> running;
        
        // Mutex for thread-safe access to kinematic data
        mutable std::mutex dataMutex;

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

        /*
        **************************
        CONTROL FUNCTIONS
        **************************
        */
        // Calculate control forces for PID
        Vec3 calculateControlForces(const Vec3& target, double dt);

        // Reset controllers
        void resetControllers();

};

// External thread function
void threadFunction(ECE_UAV* pUAV);
