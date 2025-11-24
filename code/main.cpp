/*
Authors: Aaron Huang, Dulani Wijayarathne, Matt Chung
Class: ECE 6122
Last Date Modified: 

Description:

*/

#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controlsAlternate.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <vector>

int main( void )
{
	// Initialize GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make macOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 1024, 768, "Tutorial 09 - Rendering several models", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
    
	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited movement
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Set the mouse at the center of the screen
    glfwPollEvents();
    //glfwSetCursorPos(window, 1024/2, 768/2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it is closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	// Cull triangles which normal is not towards the camera
	//glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders(
		"assets/shaders/StandardShading.vertexshader",
		"assets/shaders/StandardShading.fragmentshader"
	);

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

	// Get handles for solid color (green floor)
	GLint uUseSolid   = glGetUniformLocation(programID, "useSolidColor");
	GLint uSolidColor = glGetUniformLocation(programID, "solidColor");

	// Load the texture
	GLuint Texture = loadDDS("assets/textures/uvmap.DDS");

	// Load floor texture
	GLuint floorTexture = loadDDS("assets/textures/cat.DDS");

	// Set up texture parameters
	glBindTexture(GL_TEXTURE_2D, floorTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

	// Bind our texture in Texture Unit 0
	glUseProgram(programID);        
	glUniform1i(TextureID, 0);    




	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ("assets/models/suzanne.obj", vertices, uvs, normals);

	std::vector<unsigned short> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);

	// Load it into a VBO

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);

	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

	// Generate a buffer for the indices as well
	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0] , GL_STATIC_DRAW);

	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;

	// For vector initialization
	int numberObjects = 8;
	std::vector<glm::mat4> modelMatrices(numberObjects);
	std::vector<glm::mat4> MVPMatrices(numberObjects);

	// For Rotation and Translation
	static float rotationAngle = 360.0f / (float)numberObjects;
	static float radius = 3.65f;
	//static float radius = 5.65f;
	float normalAngle;
	float currentAngle = 0.0f;
	float radNormalAngle;
	const float s = radius + 1.0f;

	// Create floor and its UVs
	GLfloat floorVerts[] = 
	{
		// tri 1
		-s,0,-s,   -s,0, s,    s,0,-s,
		// tri 2
		s,0,-s,   -s,0, s,    s,0, s,
	};
	GLfloat floorUVs[] = 
	{
		0.f,0.f,  0.f,1.f,  1.f,0.f,
		1.f,0.f,  0.f,1.f,  1.f,1.f,
	};

	// Load floor into VBOs
	GLuint floorVBO;
	glGenBuffers(1, &floorVBO);
	glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floorVerts), floorVerts, GL_STATIC_DRAW);

	GLuint floorUV_VBO;
	glGenBuffers(1, &floorUV_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, floorUV_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floorUVs), floorUVs, GL_STATIC_DRAW);

	// Enable toggling of direct light
	GLint uEnableDirectLoc = glGetUniformLocation(programID, "uEnableDirect");
	bool enableDirect = true;
	int lastL = GLFW_RELEASE;

	do{
		// Update light toggle
		int L = glfwGetKey(window, GLFW_KEY_L);
		if (L == GLFW_PRESS && lastL == GLFW_RELEASE) {
			enableDirect = !enableDirect;          // toggle on key press
		}
		lastL = L;

		glUseProgram(programID);
		glUniform1i(uEnableDirectLoc, enableDirect ? 1 : 0);

		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		if ( currentTime - lastTime >= 1.0 ){ // If last prinf() was more than 1sec ago
			// printf and reset
			printf("%f ms/frame\n", 1000.0/double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		
		
		// Use our shader
		glUseProgram(programID);
		glm::vec3 lightPos = glm::vec3(7,7,7);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"
		
		/////// Create Green Floor ////////
		glDisable(GL_CULL_FACE);
		glm::mat4 modelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * modelMatrix;

		// Set solid color to true (green)
		//glUniform1i(uUseSolid, GL_TRUE);
		//glUniform3f(uSolidColor, 0.0f, 1.0f, 0.0f); 

		// Set floor texture
		glUniform1i(uUseSolid, GL_FALSE);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, floorTexture);

		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &modelMatrix[0][0]);

		// draw:
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, floorUV_VBO);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Make UVs constant (no array), e.g., sample center of texture
		// This was for green floor with solid color
		// glDisableVertexAttribArray(1);
		// glVertexAttrib2f(1, 0.5f, 0.5f);

		// Make normals constant up (+Y) for both tris
		glDisableVertexAttribArray(2);
		glVertexAttrib3f(2, 0.0f, 1.0f, 0.0f);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		/////// End of Green Floor ////////


		/////// NEW MATRIX TO RENDER ALL OBJECTS ////////
		glEnable(GL_CULL_FACE);

		// Disable solid color for objects
		glUseProgram(programID);
		glUniform1i(uUseSolid, GL_FALSE);   

		glUseProgram(programID);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);

		// For loop to draw the suzanne objects
		for (int object = 0; object < numberObjects; ++object)
		{
			// define model matrix and parameters
			modelMatrices[object] = glm::mat4(1.0);
			normalAngle = currentAngle + rotationAngle * object;
			radNormalAngle = glm::radians(normalAngle);

			// Rotation
			modelMatrices[object] = glm::rotate(modelMatrices[object], radNormalAngle, glm::vec3(0.0f, 1.0f, 0.0f));
			// Translation
			modelMatrices[object] = glm::translate(modelMatrices[object], glm::vec3(0.0f, 1.0f, radius));

			// Update MVP matrix
			MVPMatrices[object] = ProjectionMatrix * ViewMatrix * modelMatrices[object];

			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVPMatrices[object][0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &modelMatrices[object][0][0]);

			// The rest is exactly the same as the first object
		
			// 1rst attribute buffer : vertices
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			// 2nd attribute buffer : UVs
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

			// 3rd attribute buffer : normals
			glEnableVertexAttribArray(2);
			glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			// Index buffer
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

			// Draw the triangles !
			glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void*)0);
		}

		/////// END OF NEW MATRIX //////////

		// Disable vertex arrays
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteBuffers(1, &elementbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &Texture);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}