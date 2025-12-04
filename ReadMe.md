# Autonomous UAV Swarm Simulation

## Overview
This project simulates a swarm of 15 autonomous Unmanned Aerial Vehicles (UAVs) performing a coordinated flight routine on a virtual football field. The simulation utilizes **OpenGL** for rendering and **multithreading** to simulate independent physics calculations for each drone in real-time.

The core of the simulation relies on a **PID Controller** to navigate drones from a ground formation to a high-altitude target, where they orbit a central sphere before returning to their starting positions.

## Key Features

### Physics & Control
* **Multithreaded Architecture:** Each UAV runs on its own dedicated thread (15 physics threads + 1 rendering thread), ensuring independent kinematic updates every 10ms.
* **PID Control System:** Implements a Proportional-Integral-Derivative controller to handle flight stability, altitude maintenance, and orbit corrections against gravity.
* **Physics Engine:** Custom kinematic solver handling:
    * Newtonian mechanics ($F=ma$)
    * Gravity compensation (10N constant force)
    * Velocity and acceleration limits
* **Collision Detection:** Real-time elastic collision handling between UAVs using thread-safe atomic velocity swaps.

### Visuals & Graphics
* **Diverse Fleet:** Renders 3 distinct 3D object models (Suzanne, Cube, Chicken) with unique texture maps.
* **Dynamic Light Trails:** Renders visual flight paths that dynamically sample the texture/color of the specific UAV model.
* **Spinning Animations:** UAVs rotate on their local axis to simulate propeller torque/flight stability.
* **Textured Environment:** Includes a texture-mapped football field and generic background environments.

### Interactive Controls
The simulation includes a chase-camera system allowing the user to inspect individual drones.

| Key | Action |
| :--- | :--- |
| **W, A, S, D** | Pan and Move Camera (Free Roam) |
| **Arrow Keys** | Cycle through different UAVs |
| **C** | Toggle "Chase Cam" mode (Follow selected UAV) |
| **L** | Toggle Lighting |
| **ESC** | Exit Simulation |

## Simulation Sequence
1.  **Idle Phase:** Drones initialize on the yard lines of the football field (0, 25, 50, -25, -50).
2.  **Launch Phase:** After 5 seconds, the swarm launches simultaneously toward a central convergence point (0, 0, 50).
3.  **Orbit Phase:** Drones enter a random orbit pattern around a 10m radius sphere, maintaining velocities between 2m/s and 10m/s using PID corrections.
4.  **Return Phase:** Upon completing the required flight duration, drones return to their original coordinates.

## Build Instructions

### Prerequisites
* CMake (3.10 or higher)
* OpenGL 3.3+
* GLFW3
* GLEW

### Compiling and Running
Navigate to the build directory and generate the Makefiles using CMake:

```bash
cd build
cmake ..
cmake --build . -j 16
