/*
Author: Matt Chung
Class: ECE6122
Last Date Modified: November 29, 2025

Description:
Global physics parameters and functions for UAV simulation.
*/

#pragma once
#include <vector>
#include "ECE_UAV.h"

// Forward declaration to avoid circular dependency
class ECE_UAV; 

// A global pointer for accessing all UAVs in the simulation
extern std::vector<ECE_UAV*>* GLOBAL_UAV_LIST;

// Function to check for collisions between UAVs
void checkCollisionsFor(ECE_UAV* me);

// Configure the UAV collision bounding radius (in meters)
void setUAVBoundingRadius(double radius);