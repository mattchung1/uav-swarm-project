/*
Header file for ECE_UAV class

*/


#pragma once
#include <thread>
#include <atomic>
#include <mutex>

class ECE_UAV {
    private:
        // Mass of UAV (1 kg)
        double mass;
        
        // Position (x, y, z)
        double x, y, z;
        
        // Velocity (vx, vy, vz)
        double vx, vy, vz;
        
        // Acceleration (ax, ay, az)
        double ax, ay, az;
        
        // Thread for kinematic updates
        std::thread uavThread;
        
        // Flag to control thread execution
        std::atomic<bool> running;
        
        // Mutex for thread-safe access to kinematic data
        std::mutex dataMutex;

    public:
        //Declare member functions
        ECE_UAV(double startX, double startY, double startZ);
        ~ECE_UAV();

        // Start the thread running threadFunction
        void start();
        
        // Stop the thread
        void stop();
        
        // Get position of UAV (thread-safe)
        void getPosition(double& outX, double& outY, double& outZ);
        
        // Get velocity of UAV (thread-safe)
        void getVelocity(double& outVx, double& outVy, double& outVz);
        
        // Get acceleration of UAV (thread-safe)
        void getAcceleration(double& outAx, double& outAy, double& outAz);

        // Update kinematics (called by threadFunction)
        void updateKinematics(double deltaTime);
        
        // Set acceleration (for external forces)
        void setAcceleration(double newAx, double newAy, double newAz);
        
        // Check if thread is running
        bool isRunning() const { return running; }
        
        // Friend function declaration
        friend void threadFunction(ECE_UAV* pUAV);
};

// External thread function
void threadFunction(ECE_UAV* pUAV);
