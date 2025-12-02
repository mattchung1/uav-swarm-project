/*
Author: Dulani Wijayarathne
Class: ECE6122
Last Date Modified: November 29, 2025

Description:
PID (Proportional-Integral-Derivative) Controller class for maintaining UAV flight path.
This controller calculates corrective forces to minimize error between current state
and desired setpoint.
*/

#pragma once

class PIDController 
{
private:
    // PID gains
    double kp; // Proportional gain
    double ki; // Integral gain
    double kd; // Derivative gain
    
    // State variables
    double integral;        // Accumulated error over time
    double previousError;   // Error from last update
    
    // Anti-windup limits for integral term
    double integralMax;
    double integralMin;
    
public:
    /*
    Constructor for PID Controller
    Input: 
        - proportional: Kp gain (corrects based on current error)
        - integral: Ki gain (corrects based on accumulated error)
        - derivative: Kd gain (corrects based on rate of error change)
    */
    PIDController(double proportional = 1.0, double integral = 0.0, double derivative = 0.0);
    
    /*
    Calculate PID control output
    Input:
        - setpoint: Desired value (target state)
        - processVariable: Current measured value (actual state)
        - deltaTime: Time elapsed since last update (seconds)
    Output:
        - Control signal to apply to system
    */
    double calculate(double setpoint, double processVariable, double deltaTime);
    
    /*
    Reset the PID controller state
    Clears integral accumulation and previous error
    */
    void reset();
    
    /*
    Set new PID gains
    Input:
        - proportional: New Kp value
        - integral: New Ki value
        - derivative: New Kd value
    */
    void setGains(double proportional, double integral, double derivative);
    
    /*
    Set limits for integral term to prevent windup
    Input:
        - min: Minimum integral value
        - max: Maximum integral value
    */
    void setIntegralLimits(double min, double max);
};
