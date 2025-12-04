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

## Mathematical Model & Control Logic

The simulation relies on a custom physics engine and control loop running at **100Hz** ($\Delta t = 0.01s$). Below is the governing logic for the UAV flight behavior.

### 1. PID Controller Implementation
To navigate the UAVs autonomously from the ground to the orbital sphere, a Proportional-Integral-Derivative (PID) controller is applied to the error vector $\vec{e}(t)$, defined as the difference between the target position $\vec{P}_{target}$ and current position $\vec{P}_{current}$.

$$\vec{e}(t) = \vec{P}_{target} - \vec{P}_{current}$$

The control output $\vec{u}(t)$ (Thrust) is calculated as:

$$\vec{u}(t) = K_p \vec{e}(t) + K_i \int_{0}^{t} \vec{e}(\tau) d\tau + K_d \frac{d\vec{e}(t)}{dt}$$

Where:
* **$K_p$ (Proportional):** Provides immediate reaction to position error.
* **$K_i$ (Integral):** Corrects steady-state error (e.g., wind or drift accumulation).
* **$K_d$ (Derivative):** Dampens the motion to prevent overshooting the target coordinates.

### 2. Physics Integration
The simulation utilizes semi-implicit Euler integration to update the kinematic state of each drone. The net force $\vec{F}_{net}$ acting on a UAV is the sum of the PID control thrust, gravity compensation, and external forces.

$$\vec{F}_{net} = \vec{u}(t) + \vec{F}_{gravity} + \vec{F}_{collision}$$

Given a constant mass $m$ and a gravity compensation force of $10N$ (opposing $-9.8 m/s^2$):

$$\vec{a}_{t} = \frac{\vec{F}_{net}}{m}$$

The state update for the next time step $t+1$:

$$\vec{v}_{t+1} = \vec{v}_t + \vec{a}_t \cdot \Delta t$$
$$\vec{P}_{t+1} = \vec{P}_t + \vec{v}_{t+1} \cdot \Delta t$$

### 3. Collision Resolution (Elastic)
Collision detection is performed via bounding sphere checks between all pairs of UAVs. If the distance $d$ between two UAVs ($A$ and $B$) falls below the sum of their radii:

$$\|\vec{P}_A - \vec{P}_B\| < 2 \cdot r_{drone}$$

The system treats the collision as perfectly elastic. To simplify the computational load in the multithreaded environment, velocity vectors are atomically swapped or reflected based on the incidence angle, conserving system momentum:

$$\vec{v}_{A, new} \approx \vec{v}_{B, old} \quad , \quad \vec{v}_{B, new} \approx \vec{v}_{A, old}$$

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
