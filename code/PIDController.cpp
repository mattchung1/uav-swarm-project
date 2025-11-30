/*
Author: Person 3
Class: ECE6122
Last Date Modified: November 29, 2025

Description:
Implementation of PID Controller class. Provides proportional, integral, and derivative
control for maintaining UAV flight characteristics.
*/

#include "PIDController.h"
#include <algorithm>

/*
Constructor: Initialize PID controller with gains
*/
PIDController::PIDController(double proportional, double integral, double derivative)
    : kp(proportional), ki(integral), kd(derivative),
      integral(0.0), previousError(0.0),
      integralMax(100.0), integralMin(-100.0)
{
}

/*
Calculate PID output based on error between setpoint and current value
*/
double PIDController::calculate(double setpoint, double processVariable, double deltaTime)
{
    // Calculate error (difference between desired and actual)
    double error = setpoint - processVariable;
    
    // Proportional term: Kp * error
    // Responds immediately to current error
    double proportionalTerm = kp * error;
    
    // Integral term: Ki * sum(error * dt)
    // Addresses persistent steady-state errors
    integral += error * deltaTime;
    
    // Anti-windup: Clamp integral to prevent excessive accumulation
    integral = std::max(integralMin, std::min(integralMax, integral));
    
    double integralTerm = ki * integral;
    
    // Derivative term: Kd * (error - previousError) / dt
    // Predicts future error and dampens oscillations
    double derivative = 0.0;
    if (deltaTime > 0) 
    {
        derivative = (error - previousError) / deltaTime;
    }
    double derivativeTerm = kd * derivative;
    
    // Store current error for next iteration
    previousError = error;
    
    // Calculate total control output
    double output = proportionalTerm + integralTerm + derivativeTerm;
    
    return output;
}

/*
Reset controller state - useful when switching control modes
*/
void PIDController::reset()
{
    integral = 0.0;
    previousError = 0.0;
}

/*
Update PID gains during runtime
*/
void PIDController::setGains(double proportional, double integral, double derivative)
{
    kp = proportional;
    ki = integral;
    kd = derivative;
}

/*
Set limits to prevent integral windup
*/
void PIDController::setIntegralLimits(double min, double max)
{
    integralMin = min;
    integralMax = max;
}
