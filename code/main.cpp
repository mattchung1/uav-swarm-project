/*
Authors: Aaron Huang, Dulani Wijayarathne, Matt Chung
Class: ECE 6122
Last Date Modified: November 30th, 2025
Description: Final Project

Project Statement of work:
After completing the default final project together as a group, we later realized that it was supposed to be a project completed individually. We 
incorrectly assumed that both the default and custom final projects could be completed either as groups or as an individual. As per an email with Prof. Hurley, 
we compromised by adding more complexity to the default final project and submitting it as a group. The added complexity is as follows:

1. Each UAV outputs a colored trail
2. Three different UAV types with unique textures
3. Spinning UAVs
4. Keyboard controls

*/

#include <stdio.h>
#include <stdlib.h>
#include <array>
#include <vector>
#include <iostream>
#include <limits>
#include <algorithm>

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
#include "ECE_UAV.h"
#include "Vec3.h"
#include "PhysicsGlobals.h"

// Adding function for window resize
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // Prevent a divide by zero if the window is too small
    if (height == 0) height = 1;
    
    // Tell OpenGL to use the full window
    glViewport(0, 0, width, height);
}


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
	window = glfwCreateWindow( 400, 400, "Tutorial 09 - Rendering several models", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Frame resize callback
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
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

	// Enable blending for transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
	GLint uSolidAlpha = glGetUniformLocation(programID, "solidAlpha");
	GLint uColorIntensityLoc = glGetUniformLocation(programID, "uColorIntensity");

	// Load UAV textures for each model group with sensible fallbacks
	GLuint texture0 = loadDDS("assets/textures/txtr01.DDS");
	if (!texture0) {
		texture0 = loadTextureWithStb("assets/textures/txtr01.DDS");
	}
	if (!texture0) {
		texture0 = loadTextureWithStb("assets/textures/txtr01.jpg");
	}
	if (!texture0) {
		fprintf(stderr, "Failed to load txtr01; falling back to assets/textures/cat.DDS.\n");
		texture0 = loadDDS("assets/textures/cat.DDS");
	}
	if (!texture0) {
		fprintf(stderr, "Unable to load any UAV texture for group 0.\n");
		return -1;
	}

	GLuint texture1 = loadTextureWithStb("assets/textures/redtexture.jpg");
	if (!texture1) {
		texture1 = loadDDS("assets/textures/redtexture.DDS");
	}
	if (!texture1) {
		fprintf(stderr, "Failed to load redtexture; falling back to txtr01/cat for group 1.\n");
		texture1 = texture0; // fallback to texture0 which has valid content
	}

	GLuint texture2 = loadTextureWithStb("assets/textures/whitemarble.jpg");
	if (!texture2) {
		// Try a DDS or fallback to texturuue0/cat
		texture2 = loadDDS("assets/textures/whitemarble.DDS");
	}
	if (!texture2) {
		fprintf(stderr, "Failed to load whitemarble.jpg; falling back to txtr01/cat for group 2.\n");
		texture2 = texture0;
	}

	// Load floor texture
	GLuint floorTexture = loadBMP_custom("assets/textures/ff.bmp");

	// Set up texture parameters
	glBindTexture(GL_TEXTURE_2D, floorTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");
	GLint uColorIntensityID = glGetUniformLocation(programID, "uColorIntensity");

	// Bind our default texture in Texture Unit 0 (unimportant; we'll bind per-model later)
	glUseProgram(programID);
	glUniform1i(TextureID, 0);
	glUniform1f(uColorIntensityID, 1.0f);




	// Generic model resources container
	struct ModelResources {
		GLuint vbo = 0;
		GLuint uvbo = 0;
		GLuint nbo = 0;
		GLuint ebo = 0;
		std::vector<unsigned short> idx;
		float scale = 1.0f;
	};

	auto loadIndexed = [](const char* path,
						  std::vector<glm::vec3>& outVerts,
						  std::vector<glm::vec2>& outUVs,
						  std::vector<glm::vec3>& outNormals,
						  std::vector<unsigned short>& outIdx,
						  std::vector<glm::vec3>& outIndexedVerts) -> bool {
		std::vector<glm::vec3> v;
		std::vector<glm::vec2> u;
		std::vector<glm::vec3> n;
		if (!loadOBJ(path, v, u, n)) {
			fprintf(stderr, "Failed to load %s\n", path);
			return false;
		}
		indexVBO(v, u, n, outIdx, outIndexedVerts, outUVs, outNormals);
		outVerts.swap(v);
		return true;
	};

	// Load three models: UAV1, UAV2, UAV3
	std::vector<glm::vec3> uav1Verts, uav1IndexedVerts; // first 5
	std::vector<glm::vec2> uav1UVs;
	std::vector<glm::vec3> uav1Normals;
	std::vector<unsigned short> uav1Idx;
	if (!loadIndexed("assets/models/suzanne.obj", uav1Verts, uav1UVs, uav1Normals, uav1Idx, uav1IndexedVerts)) {
		return -1;
	}

	std::vector<glm::vec3> uav2Verts, uav2IndexedVerts; // next 5
	std::vector<glm::vec2> uav2UVs;
	std::vector<glm::vec3> uav2Normals;
	std::vector<unsigned short> uav2Idx;
	if (!loadIndexed("assets/models/cube.obj", uav2Verts, uav2UVs, uav2Normals, uav2Idx, uav2IndexedVerts)) {
		return -1;
	}

	std::vector<glm::vec3> uav3Verts, uav3IndexedVerts; // last 5
	std::vector<glm::vec2> uav3UVs;
	std::vector<glm::vec3> uav3Normals;
	std::vector<unsigned short> uav3Idx;
	if (!loadIndexed("assets/models/chicken_01.obj", uav3Verts, uav3UVs, uav3Normals, uav3Idx, uav3IndexedVerts)) {
		return -1;
	}

		// Set physics collision radius target to match rendered drone size
		const float desiredBoundingBoxMeters = 0.2f; // Physical requirement (20 cm cube)
		const float visualScaleMultiplier = 10.0f;   // Visibility multiplier
		const float uavBoundingRadiusMeters = 0.5f * desiredBoundingBoxMeters * visualScaleMultiplier;

		// Update physics collision radius to match rendered drone size
		setUAVBoundingRadius(uavBoundingRadiusMeters);

	// (Legacy single-model buffers removed; using generic model groups below)

	// Build buffers for three UAV model groups
	ModelResources models[3];

	auto uploadBuffers = [](ModelResources& mr,
							const std::vector<glm::vec3>& verts,
							const std::vector<glm::vec2>& uvs,
							const std::vector<glm::vec3>& norms,
							const std::vector<unsigned short>& idx) {
		glGenBuffers(1, &mr.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, mr.vbo);
		glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(glm::vec3), &verts[0], GL_STATIC_DRAW);

		glGenBuffers(1, &mr.uvbo);
		glBindBuffer(GL_ARRAY_BUFFER, mr.uvbo);
		glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

		glGenBuffers(1, &mr.nbo);
		glBindBuffer(GL_ARRAY_BUFFER, mr.nbo);
		glBufferData(GL_ARRAY_BUFFER, norms.size() * sizeof(glm::vec3), &norms[0], GL_STATIC_DRAW);

		glGenBuffers(1, &mr.ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mr.ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned short), &idx[0], GL_STATIC_DRAW);
		mr.idx = idx;
	};

	uploadBuffers(models[0], uav1IndexedVerts, uav1UVs, uav1Normals, uav1Idx);
	uploadBuffers(models[1], uav2IndexedVerts, uav2UVs, uav2Normals, uav2Idx);
	uploadBuffers(models[2], uav3IndexedVerts, uav3UVs, uav3Normals, uav3Idx);

	// Compute per-model scales to match the same physical target
	auto computeScale = [](const std::vector<glm::vec3>& indexedVerts) -> float {
		glm::vec3 minB(std::numeric_limits<float>::max());
		glm::vec3 maxB(std::numeric_limits<float>::lowest());
		for (const auto& v : indexedVerts) {
			minB.x = std::min(minB.x, v.x);
			minB.y = std::min(minB.y, v.y);
			minB.z = std::min(minB.z, v.z);
			maxB.x = std::max(maxB.x, v.x);
			maxB.y = std::max(maxB.y, v.y);
			maxB.z = std::max(maxB.z, v.z);
		}
		float ex = maxB.x - minB.x, ey = maxB.y - minB.y, ez = maxB.z - minB.z;
		float m = std::max(std::max(ex, ey), ez);
		const float desiredBoundingBoxMeters = 0.2f;
		const float visualScaleMultiplier = 10.0f;
		const float base = m > 0.0f ? desiredBoundingBoxMeters / m : 1.0f;
		return base * visualScaleMultiplier;
	};

	// Compute each model's scale so all share the same physical size
	models[0].scale = computeScale(uav1IndexedVerts);
	models[1].scale = computeScale(uav2IndexedVerts);
	models[2].scale = computeScale(uav3IndexedVerts);

	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;

	// For 30ms UAV polling
	double lastPollTime = glfwGetTime();
	const double pollInterval = 0.030; // 30 milliseconds

	// For vector initialization - 15 UAVs for multithreading
	const int numberUAVs = 15;
	std::vector<glm::mat4> modelMatrices(numberUAVs);
	std::vector<glm::mat4> MVPMatrices(numberUAVs);

	// Trail colors initialization
	const int TRAIL_LENGTH = 100;
	std::vector<std::vector<glm::vec3>> uavTrails(numberUAVs);
	GLuint trailVBO;
	glGenBuffers(1, &trailVBO);

	// Create 15 ECE_UAV objects placed on football yard lines using a 3x5 grid
	std::vector<ECE_UAV*> uavs;
	GLOBAL_UAV_LIST = &uavs;

	const float halfFieldLengthYards = 50.0f;          // Goal line to midfield
	const float halfFieldLengthUnits = 128.0f;         // Matches floorDepth below
	const float yardsToUnits = halfFieldLengthUnits / halfFieldLengthYards;

	const float halfFieldWidthYards = 26.6667f;        // 160 ft / 2 converted to yards
	const float halfFieldWidthUnits = 256.0f;          // Matches floorWidth below
	const float lateralScale = halfFieldWidthUnits / halfFieldWidthYards;

	const std::array<float, 3> yardLineOffsetsYards = {-43.f, 0.f, 43.f};
	const std::array<float, 5> lateralOffsetsYards = {-20.f, -10.f, 0.f, 10.f, 20.f};

	std::vector<Vec3> formationPositions;
	formationPositions.reserve(yardLineOffsetsYards.size() * lateralOffsetsYards.size());
	for (float yardLine : yardLineOffsetsYards) {
		float y = yardLine * yardsToUnits;
		for (float lateral : lateralOffsetsYards) {
			float x = lateral * lateralScale;
			formationPositions.emplace_back(x, y, 0.0f); // 
		}
	}

	if (formationPositions.size() < static_cast<size_t>(numberUAVs)) {
		fprintf(stderr, "Formation definition does not cover all UAVs.\n");
		return -1;
	}

	for (int i = 0; i < numberUAVs; ++i) {
		uavs.push_back(new ECE_UAV(formationPositions[i]));
	}

	// Start all UAV threads
	for (int i = 0; i < numberUAVs; ++i) {
		uavs[i]->start();
	}

	// Position storage for UAVs (used in render loop)
	std::vector<Vec3> currentPos(numberUAVs);

	// --- ADD THIS TO FIX INVISIBILITY ON FRAME 1 ---
    for (int i = 0; i < numberUAVs; ++i) {
        currentPos[i] = uavs[i]->getPosition(); 
    }

	// For Rotation and Translation
	static float rotationAngle = 360.0f / (float)numberUAVs;
	static float radius = 3.65f;
	//static float radius = 5.65f;
	float normalAngle;
	float currentAngle = 0.0f;
	float radNormalAngle;
	const float floorWidth = 256.0f;   // Football field width
	const float floorDepth = 128.0f;   // Football field length (512/2 for half-extents)

	// Create floor and its UVs (X-Y plane at Z=0 for Z-up coordinate system)
	GLfloat floorVerts[] = 
	{
		// tri 1
		-floorWidth,-floorDepth,0,   -floorWidth,floorDepth,0,    floorWidth,-floorDepth,0,
		// tri 2
		floorWidth,-floorDepth,0,   -floorWidth,floorDepth,0,    floorWidth,floorDepth,0,
	};
	GLfloat floorUVs[] = 
	{
		// Flipped back to normal orientation for Z-up coordinate system
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

	// Create sphere geometry for target visualization
	const float sphereRadius = 10.0f;
	const int sphereStacks = 20;
	const int sphereSlices = 20;
	std::vector<glm::vec3> sphereVertices;
	std::vector<glm::vec3> sphereNormals;
	std::vector<unsigned short> sphereIndices;

	// Generate sphere vertices
	for (int i = 0; i <= sphereStacks; ++i) {
		float V = i / (float)sphereStacks;
		float phi = V * glm::pi<float>();

		for (int j = 0; j <= sphereSlices; ++j) {
			float U = j / (float)sphereSlices;
			float theta = U * 2.0f * glm::pi<float>();

			float x = cos(theta) * sin(phi);
			float y = cos(phi);
			float z = sin(theta) * sin(phi);

			sphereVertices.push_back(glm::vec3(x, y, z) * sphereRadius);
			sphereNormals.push_back(glm::vec3(x, y, z));
		}
	}

	// Generate sphere indices
	for (int i = 0; i < sphereStacks; ++i) {
		for (int j = 0; j < sphereSlices; ++j) {
			int first = i * (sphereSlices + 1) + j;
			int second = first + sphereSlices + 1;

			sphereIndices.push_back(first);
			sphereIndices.push_back(second);
			sphereIndices.push_back(first + 1);

			sphereIndices.push_back(second);
			sphereIndices.push_back(second + 1);
			sphereIndices.push_back(first + 1);
		}
	}

	// Load sphere into VBOs
	GLuint sphereVertexBuffer;
	glGenBuffers(1, &sphereVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(glm::vec3), &sphereVertices[0], GL_STATIC_DRAW);

	GLuint sphereNormalBuffer;
	glGenBuffers(1, &sphereNormalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphereNormalBuffer);
	glBufferData(GL_ARRAY_BUFFER, sphereNormals.size() * sizeof(glm::vec3), &sphereNormals[0], GL_STATIC_DRAW);

	GLuint sphereIndexBuffer;
	glGenBuffers(1, &sphereIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned short), &sphereIndices[0], GL_STATIC_DRAW);

	// Enable toggling of direct light
	GLint uEnableDirectLoc = glGetUniformLocation(programID, "uEnableDirect");
	bool enableDirect = true;
	int lastL = GLFW_RELEASE;

	bool simulationRunning = true;
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

		// Poll UAV positions every 30ms
		if (currentTime - lastPollTime >= pollInterval) {
			// Update positions from UAV threads and check completion state
			bool allFinished = true;
			for (int i = 0; i < numberUAVs; ++i) {
				Vec3 p = uavs[i]->getPosition();
				currentPos[i] = p; // Store position for light trails and rendering

				// For trail storage
				glm::vec3 glPos(p.x, p.y, p.z);
				uavTrails[i].insert(uavTrails[i].begin(), glPos);

				// Remove old trail points exceeding TRAIL_LENGTH
				if (uavTrails[i].size() > TRAIL_LENGTH) {
                    uavTrails[i].pop_back();
                }

				if (!uavs[i]->hasCompletedOrbit())
				{
					allFinished = false;
				}
			}
			if (allFinished)
			{
				std::cout << "All UAVs completed their orbit — ending simulation." << std::endl;
				simulationRunning = false;
			}
			lastPollTime = currentTime;
		}

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();


		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		
		
		// Use our shader
		glUseProgram(programID);
		glm::vec3 lightPos = glm::vec3(0, 200, 0);  // High overhead light at field center
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"
		
		/////// Create Green Floor ////////
		glDisable(GL_CULL_FACE);
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(-16.0f, 0.0f, 0.0f)); // offset to make the football field fit nicely with the current orientation of objects
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * modelMatrix;

		// Set solid color to true (green)
		//glUniform1i(uUseSolid, GL_TRUE);
		//glUniform3f(uSolidColor, 0.0f, 1.0f, 0.0f); 

		// Set floor texture
		glUniform1i(uUseSolid, GL_FALSE);
		glUniform1f(uColorIntensityLoc, 1.0f);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, floorTexture);

		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &modelMatrix[0][0]);
		glUniform1f(uColorIntensityID, 1.0f);

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

		// Make normals constant up (+Z) for both tris
		glDisableVertexAttribArray(2);
		glVertexAttrib3f(2, 0.0f, 0.0f, 1.0f);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		/////// End of Green Floor ////////




		/////// NEW MATRIX TO RENDER ALL OBJECTS ////////
		glEnable(GL_CULL_FACE);

		// Disable solid color for objects
		glUseProgram(programID);
		glUniform1i(uUseSolid, GL_FALSE);   

		glUseProgram(programID);

		// We'll bind textures per-model inside the draw loop (group-dependent)

		// Precompute orientation and scale so each UAV stands upright and matches physics bounds
		const glm::mat4 uavOrientation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		// Z-axis spin angle based on time
		float spinAngle = glm::radians(fmod(currentTime * 360.0, 360.0)); // 50 degrees per second

		// For loop to draw the UAVs (first 5 replaced by UAV2: suzanne)
		for (int object = 0; object < numberUAVs; ++object)
		{
			// Get current position from UAV thread
			double x, y, z;

			x = currentPos[object].x;
			y = currentPos[object].y;
			z = currentPos[object].z;
			
			// Select which model group this UAV belongs to: 0 (0-4), 1 (5-9), 2 (10-14)
			int group = (object < 5) ? 0 : (object < 10) ? 1 : 2;
			const ModelResources& mr = models[group];

			// define model matrix and parameters
			modelMatrices[object] = glm::mat4(1.0);
			float colorIntensity = static_cast<float>(uavs[object]->getColorIntensity());
			glUniform1f(uColorIntensityLoc, colorIntensity);
			// Translate to UAV position, then orient upright, spin around Z-axis, and scale down with per-model scale
			modelMatrices[object] = glm::translate(modelMatrices[object], glm::vec3((float)x, (float)y, (float)z));
			modelMatrices[object] = modelMatrices[object] * uavOrientation;
			modelMatrices[object] = glm::rotate(modelMatrices[object], spinAngle, glm::vec3(0.0f, 1.0f, 0.0f));
			modelMatrices[object] = glm::scale(modelMatrices[object], glm::vec3(mr.scale));

			// Update MVP matrix
			MVPMatrices[object] = ProjectionMatrix * ViewMatrix * modelMatrices[object];

			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVPMatrices[object][0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &modelMatrices[object][0][0]);

			// 1rst attribute buffer : vertices
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, mr.vbo);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			// 2nd attribute buffer : UVs
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, mr.uvbo);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

			// 3rd attribute buffer : normals
			glEnableVertexAttribArray(2);
			glBindBuffer(GL_ARRAY_BUFFER, mr.nbo);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			// Index buffer
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mr.ebo);

			// Bind texture for this model group
			GLuint boundTex = (group == 0) ? texture0 : (group == 1) ? texture1 : texture2;
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, boundTex);
			glUniform1i(TextureID, 0);

			// Draw the triangles !
			glDrawElements(GL_TRIANGLES, mr.idx.size(), GL_UNSIGNED_SHORT, (void*)0);
		}

		/////// END OF NEW MATRIX //////////


		/////// Draw Light Trails ////////
		glUseProgram(programID);

		// Use Identity Matrix for Model (World Space)
        glm::mat4 Identity = glm::mat4(1.0);
		// Reusing MVP variable
        MVP = ProjectionMatrix * ViewMatrix * Identity;
        glUniformMatrix4fv(
			MatrixID, 		// Matrix ID
			1, 				// Number of matrices
			GL_FALSE, 		// Transpose
			&MVP[0][0]);	// Pointer to first element


		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &Identity[0][0]);

		// Set color of trails
		glUniform1i(uUseSolid, GL_FALSE);
        glUniform1f(uColorIntensityLoc, 1.0f);

		// Make normals constant up
		glDisableVertexAttribArray(2);
        glVertexAttrib3f(2, 0.0f, 0.0f, 1.0f);

		// Create line segments for each trail
		for(int i=0; i<numberUAVs; i++) 
		{
            if(uavTrails[i].size() < 2) continue; // Need 2 points to make a line

			// Determine which texture to use based on UAV group
            int group = (i < 5) ? 0 : (i < 10) ? 1 : 2;
            GLuint boundTex = (group == 0) ? texture0 : (group == 1) ? texture1 : texture2;

			// Bind those textures
			glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, boundTex);
            glUniform1i(TextureID, 0);

			// Create vectors for line segments
			std::vector<glm::vec3> trailVerts;
            std::vector<glm::vec2> trailUVs;

			// Populate trail positions and UV vectors
			for (size_t k = 0; k < uavTrails[i].size(); k++) 
			{
				// Position
				trailVerts.push_back(uavTrails[i][k]);

				// Generate UVs
				float u = (float)k / (float)(uavTrails[i].size() - 1);
                trailUVs.push_back(glm::vec2(u, 0.5f));
			}

			// Generate vertices
			glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, trailVBO);
            glBufferData(GL_ARRAY_BUFFER, trailVerts.size() * sizeof(glm::vec3), &trailVerts[0], GL_DYNAMIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			// Generate UVs
			GLuint trailUVBuffer; 
            glGenBuffers(1, &trailUVBuffer);
            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, trailUVBuffer);
            glBufferData(GL_ARRAY_BUFFER, trailUVs.size() * sizeof(glm::vec2), &trailUVs[0], GL_DYNAMIC_DRAW);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

			// Draw line strip
			glDrawArrays(GL_LINE_STRIP, 0, trailVerts.size());
			
			// Clean up temporary UV buffer
			glDeleteBuffers(1, &trailUVBuffer);
        }

		/////// END OF LIGHT TRAILS //////////


		/////// Draw Semi-Transparent Target Sphere ////////
		// Position: (0, 0, 50) in Z-up coordinate system
		glm::mat4 sphereModelMatrix = glm::mat4(1.0);
		sphereModelMatrix = glm::translate(sphereModelMatrix, glm::vec3(0.0f, 0.0f, 50.0f));
		glm::mat4 sphereMVP = ProjectionMatrix * ViewMatrix * sphereModelMatrix;

		// Use solid color for sphere (semi-transparent cyan/blue)
		glUniform1i(uUseSolid, GL_TRUE);
		glUniform1f(uColorIntensityLoc, 1.0f);
		glUniform3f(uSolidColor, 0.3f, 0.7f, 1.0f);
		glUniform1f(uSolidAlpha, 0.3f);  // 30% opacity for transparency

		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &sphereMVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &sphereModelMatrix[0][0]);
		glUniform1f(uColorIntensityID, 1.0f);

		// Draw sphere
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, sphereVertexBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glDisableVertexAttribArray(1);
		glVertexAttrib2f(1, 0.5f, 0.5f);

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, sphereNormalBuffer);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereIndexBuffer);
		glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_SHORT, (void*)0);

		// Reset alpha to opaque for other objects
		glUniform1f(uSolidAlpha, 1.0f);

		/////// End of Target Sphere ////////


		// Disable vertex arrays
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glUniform1f(uColorIntensityID, 1.0f);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( simulationRunning &&
		  glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		  glfwWindowShouldClose(window) == 0 );

	// Stop all UAV threads
	for (int i = 0; i < numberUAVs; ++i) {
		uavs[i]->stop();
		delete uavs[i];
	}

	// Cleanup VBO and shader
	// Cleanup buffers for all model groups
	glDeleteBuffers(1, &models[0].vbo);
	glDeleteBuffers(1, &models[0].uvbo);
	glDeleteBuffers(1, &models[0].nbo);
	glDeleteBuffers(1, &models[0].ebo);
	glDeleteBuffers(1, &models[1].vbo);
	glDeleteBuffers(1, &models[1].uvbo);
	glDeleteBuffers(1, &models[1].nbo);
	glDeleteBuffers(1, &models[1].ebo);
	glDeleteBuffers(1, &models[2].vbo);
	glDeleteBuffers(1, &models[2].uvbo);
	glDeleteBuffers(1, &models[2].nbo);
	glDeleteBuffers(1, &models[2].ebo);
	glDeleteProgram(programID);
	glDeleteTextures(1, &texture0);
	glDeleteTextures(1, &texture1);
	glDeleteTextures(1, &texture2);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

