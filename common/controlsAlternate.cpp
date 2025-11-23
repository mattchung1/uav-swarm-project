// Include GLFW
#include <GLFW/glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controlsAlternate.hpp"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix(){
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix(){
	return ProjectionMatrix;
}


// Initial position : on +Z
glm::vec3 position = glm::vec3( 0, 3, 5 ); 
// Initial horizontal angle : toward -Z
float horizontalAngle = 3.00f;
// Initial vertical angle : none
float verticalAngle = -0.3f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 3.0f; // 3 units / second
float mouseSpeed = 0.00005f;
float radius = 10.0f;
float theta = 45.0f;
float phi = 30.0f;
//float deltaRadius = 0.01f;
//float deltaAngle = 0.1f;
float deltaRadius = 10.0f;
float deltaAngle = 50.0f;


void computeMatricesFromInputs(){
	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	// clamp frame rate to reduce jitter 
	deltaTime = glm::clamp(deltaTime, 1.0f/240.0f, 1.0f/30.0f);

	// Position and Origin vectors
	glm::vec3 origin = glm::vec3(0,0,0);

	// Up vector
	glm::vec3 up = glm::vec3(0,1,0);


	// Move forward
	if (glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS){
		//radius -= deltaRadius;
		radius -= deltaRadius * deltaTime;
	}
	// Move backward
	if (glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS){
		//radius += deltaRadius;
		radius += deltaRadius * deltaTime;
	}
	// Maintain Radius strafe right
	if (glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS){
		//theta -= deltaAngle;
		theta -= deltaAngle * deltaTime;
	}
	// Maintain Radius strafe left
	if (glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS){
		//theta += deltaAngle;
		theta += deltaAngle * deltaTime;
	}
	// Rotate up
	if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS){
		//phi += deltaAngle;
		phi += deltaAngle * deltaTime;
	}
	// Rotate down
	if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS){
		//phi -= deltaAngle;
		phi -= deltaAngle * deltaTime;
	}

	float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

	// Clamp the vertical angle to avoid flip at the poles and Limit radius
	phi = glm::clamp(phi, -89.9f, 89.9f);
    if (radius < 1.0f)  radius = 1.0f;

	ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);
	
	float x,y,z;

	// Spherical to Cartesian coordinates
	position.x = radius * sin(glm::radians(theta)) * cos(glm::radians(phi));
	position.y = radius * sin(glm::radians(phi));
	position.z = radius * cos(glm::radians(theta)) * cos(glm::radians(phi));

	// Camera matrix
	ViewMatrix = glm::lookAt(
		position,           // Camera is here
		origin, // and looks here : at the same position, plus "direction"
		up                  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}
