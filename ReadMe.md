# UAV Swarm Project

UAV Swarm Project repository that runs a 3D representation of a swarm of drones on a football field controlled by a PID controller. Drones start on various positions on a field, take off simultaneously after 5 seconds and start rotating around a central point for a few seconds. After each drone has made their revolutions around the sphere, they return to their starting locations on the field.

Each drone is controlled by their own individual thread, with the main program having its own thread. Collisions are handled as perfectly elastic and physics is handled with a simple algorithm that takes into account gravity, acceleration, force, velocity, and position of each of the drones. 


# How to use the program

To use the program, you navigate to the build folder and create the cmake environment

  ```command
  cmake ../
  cmake --build . -j 16
  ```

This will a runnable executable in the /build/bin/ directory called "FinalProject." You will see the drones move around their preprogrammed route from an aerial view of the field.