/*
Author: Person 3
Class: ECE6122
Last Date Modified: November 29, 2025

Description:
Example code snippet for Person 1 showing how to use the color intensity
function for ECE6122 requirement (color oscillation).
*/

// ============================================================================
// EXAMPLE: How Person 1 Should Use getColorIntensity() [ECE6122 ONLY]
// ============================================================================

// This is example code for Person 1's rendering loop
// It shows how to integrate the color oscillation feature

void renderUAVs(std::vector<ECE_UAV*>& uavList)
{
    // For each UAV in the simulation
    for (ECE_UAV* uav : uavList)
    {
        // Get current position for rendering
        Vec3 position = uav->getPosition();
        
        // ===== ECE6122 REQUIREMENT: Color Oscillation =====
        // Get the color intensity multiplier (oscillates 0.5 to 1.0)
        double colorIntensity = uav->getColorIntensity();
        
        // Define base UAV color (example: cyan)
        float baseR = 0.0f;
        float baseG = 1.0f;
        float baseB = 1.0f;
        
        // Apply oscillating intensity
        float finalR = baseR * colorIntensity;
        float finalG = baseG * colorIntensity;
        float finalB = baseB * colorIntensity;
        
        // Example OpenGL code to set color and render
        glColor3f(finalR, finalG, finalB);
        
        // Render UAV at position
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        // ... render UAV mesh here ...
        glPopMatrix();
    }
}

// ============================================================================
// ALTERNATIVE: Using Uniform Color Across All UAVs
// ============================================================================

void renderUAVsUniformColor(std::vector<ECE_UAV*>& uavList)
{
    // If all UAVs should pulse together, use intensity from first UAV
    double colorIntensity = uavList[0]->getColorIntensity();
    
    for (ECE_UAV* uav : uavList)
    {
        Vec3 position = uav->getPosition();
        
        // All UAVs use same intensity (synchronized pulsing)
        glColor3f(0.0f * colorIntensity,   // R
                  1.0f * colorIntensity,   // G
                  1.0f * colorIntensity);  // B
        
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        // ... render UAV mesh here ...
        glPopMatrix();
    }
}

// ============================================================================
// EXPLANATION
// ============================================================================

/*
Color Intensity Behavior:
-------------------------
- getColorIntensity() returns a value between 0.5 and 1.0
- The value oscillates sinusoidally at 0.5 Hz (one cycle every 2 seconds)
- Formula: intensity = 0.75 + 0.25 * sin(phase)
  - When sin(phase) = +1: intensity = 1.0 (full color)
  - When sin(phase) = -1: intensity = 0.5 (half color)
  - When sin(phase) = 0:  intensity = 0.75 (mid color)

Why 0.5 Hz?
----------
- Phase increments: 2π * 0.5 * dt per frame
- This creates 0.5 complete oscillations per second
- Period = 1/frequency = 1/0.5 = 2 seconds per cycle

Visual Effect:
-------------
The UAVs will appear to "pulse" or "breathe", getting brighter and dimmer
in a smooth, continuous manner. This creates an engaging visual effect
that demonstrates the UAVs are actively controlled.

Requirements Met:
----------------
✓ Color oscillates between full intensity (1.0) and half intensity (0.5)
✓ Frequency is approximately 0.5 Hz
✓ Oscillation is smooth and continuous
✓ Works for all 15 UAVs independently (or synchronized if desired)

Integration:
-----------
Simply multiply your base RGB color values by the colorIntensity value
returned from getColorIntensity() before passing to your rendering API
(OpenGL, DirectX, etc.)
*/

// ============================================================================
// DEBUGGING COLOR OSCILLATION
// ============================================================================

void debugColorIntensity(ECE_UAV* uav)
{
    // Print color intensity to console for verification
    double intensity = uav->getColorIntensity();
    std::cout << "Color Intensity: " << intensity << std::endl;
    
    // Should see values oscillating between 0.5 and 1.0
    // If stuck at one value, check that calculateStateBasedForce() 
    // is being called (colorPhase updated there)
}

// ============================================================================
// NOTES FOR PERSON 1
// ============================================================================

/*
1. Call getColorIntensity() in your main render loop (30 ms interval)
2. The intensity automatically updates in the UAV thread (10 ms interval)
3. Each UAV has its own phase, so they can oscillate independently or together
4. The oscillation starts immediately at simulation start
5. No special initialization needed - it's automatic in ECE_UAV constructor

Thread Safety:
-------------
getColorIntensity() is thread-safe. It uses mutex locking internally,
so you can safely call it from the main rendering thread while the UAV
control thread is running.

Performance:
-----------
The function is lightweight (just a mutex lock and a return statement).
Calling it 15 times per frame (for 15 UAVs) will have negligible impact
on performance.
*/
