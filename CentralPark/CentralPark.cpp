// Other headers
#include "stdafx.h"

#include <Windows.h>
#include <iostream>
//#include <Windows.h>

// GLEW
//#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include <SOIL\SOIL.h>
#include <vector>


// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void createGround();
void createGrass();
void createBuilding();
void createTexture(GLuint &texture, char* imageLocation);
void do_movement();
void generateBuildings();
void createSky();
// Window dimensions
const GLuint WIDTH = 1600, HEIGHT = 1200;
Shader *groundShader;
Shader * skyboxShader; 
//ground
GLuint groundVAO, groundVBO;
GLuint textureGround;
GLfloat groundWidth = 100.0f, groundHeight = 200.0f;
//sky
GLuint skyVBO, skyVAO;
//grass
GLuint grassVBO, grassVAO;
//buildings
GLuint buildingVAO, buildingVBO, instanceVBO, instanceVBO2;
int totalBuildings = 1200;
std::vector<glm::vec3> translations;
std::vector<glm::vec3> scalings;
int building1 = 200, building2 = 120, building3 = 240, building4 = 240, building5 = 400;
int building12 = building1 + building2;
int building123 = building1 + building2 + building3;
int building1234 = building1 + building2 + building3 + building4;
//textures
GLuint textureBuilding[5], textureSky, textureGrass; 
//camera
glm::vec3 cameraPos(0.0f, 0.5f, -3.0f), cameraFront(0.0f, 0.0f, 1.0f), cameraUp(0.0f, 1.0f, 0.0f);
bool keys[1024];


// The MAIN function, from here we start the application and run the game loop
int main()
{
	std::cout << "Starting GLFW context, OpenGL 3.3" << std::endl;
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); // for window resize
	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

	// Define the viewport dimensionsd
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	// Game loop

	// initialize shaders
	groundShader = new Shader("TextFiles/vertex.shader", "TextFiles/fragment.shader");
	skyboxShader = new Shader("TextFiles/skyBoxVertex.shader", "TextFiles/skyBoxFragment.shader");
	createSky();
	createGround();
	createGrass();
	createBuilding();
	glEnable(GL_DEPTH_TEST);

	while (!glfwWindowShouldClose(window))
	{
		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		do_movement();
		//Sleep(250);
		// Render
		// Clear the colorbuffer
		glClearColor(0.0f, 0.3f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		//Skybox shader used for sky, ground, grass
		skyboxShader->use();
		glm::mat4 view1;
		view1 = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		glm::mat4 projection1, model1(1.0f);
		projection1 = glm::perspective(glm::radians(45.0f), (GLfloat)WIDTH / (GLfloat)HEIGHT, 1.0f, 1000.0f);
		GLint modelLoc1 = glGetUniformLocation(skyboxShader->program, "model");
		GLint viewLoc1 = glGetUniformLocation(skyboxShader->program, "view");
		GLint projLoc1 = glGetUniformLocation(skyboxShader->program, "projection");
		glUniformMatrix4fv(viewLoc1, 1, GL_FALSE, glm::value_ptr(view1));
		glUniformMatrix4fv(projLoc1, 1, GL_FALSE, glm::value_ptr(projection1));
		glUniformMatrix4fv(modelLoc1, 1, GL_FALSE, glm::value_ptr(model1));
		
		glBindTexture(GL_TEXTURE_2D, textureGround);
		glBindVertexArray(groundVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glBindTexture(GL_TEXTURE_2D, textureSky);
		glBindVertexArray(skyVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glBindTexture(GL_TEXTURE_2D, textureGrass);
		glBindVertexArray(grassVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		//------------------------------------------------------------------------------------------------------------
		//ground shader used for buildings. Should be renamed buildingShader
		groundShader->use();

		glm::mat4 view;
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

		glm::mat4 projection, model(1.0f);
		projection = glm::perspective(glm::radians(45.0f), (GLfloat)WIDTH / (GLfloat)HEIGHT, 1.0f, 1000.0f);

		GLint modelLoc = glGetUniformLocation(groundShader->program, "model");
		GLint viewLoc = glGetUniformLocation(groundShader->program, "view");
		GLint projLoc = glGetUniformLocation(groundShader->program, "projection");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		

		glBindVertexArray(buildingVAO);

		glBindTexture(GL_TEXTURE_2D, textureBuilding[0]);
		glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, building1);

		//Drawing type 2 buildings with specific set of scaling vectors. MUST also change vertexattribute pointer for the translation
		//otherwise translations used for type 1 building will repeat for type two. Therefore we move pointer numOfBuilding1 away.
		glBindTexture(GL_TEXTURE_2D, textureBuilding[1]);
		glBindBuffer(GL_ARRAY_BUFFER,instanceVBO);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)(building1 * sizeof(glm::vec3)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO2);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)(building1 * sizeof(glm::vec3)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, building2);

		//same process for type 3 building
		glBindTexture(GL_TEXTURE_2D, textureBuilding[2]);
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)(building12 * sizeof(glm::vec3)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO2);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)(building12 * sizeof(glm::vec3)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, building3);

		// type 4 building
		glBindTexture(GL_TEXTURE_2D, textureBuilding[3]);
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)(building123 * sizeof(glm::vec3)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO2);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)(building123 * sizeof(glm::vec3)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, building4);

		// type 5 building
		glBindTexture(GL_TEXTURE_2D, textureBuilding[4]);
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)(building1234 * sizeof(glm::vec3)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO2);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)(building1234 * sizeof(glm::vec3)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, building5);

		
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
		// Swap the screen buffers
		glfwSwapBuffers(window);
	}

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
	return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	std::cout << key << std::endl;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (action == GLFW_PRESS) {
		keys[key] = true;
	}
	if (action == GLFW_RELEASE) {
		keys[key] = false;
	}
}

void createSky() {
	createTexture(textureSky, "Images/sky2.jpg");
	GLfloat y = -100.5f, y2 = 600.0f, z = 200.0f;

	GLfloat verticesSky[] = {
		// triangle 1   texture     normals 
		-groundWidth*3, y, z,		3.0f,0.0f,  0.0f,0.0f,-1.0f,
		groundWidth*3, y, z,		3.0f,3.0f,  0.0f,0.0f,-1.0f,
		-groundWidth*3, y2, z,		0.0f,3.0f,  0.0f,0.0f,-1.0f,
		groundWidth*3,  y2, z,		0.0f,0.0f,  0.0f,0.0f,-1.0f
	};

	GLuint indicesX[] = {
		0,1,2,
		2,3,1
	};

	glGenVertexArrays(1, &skyVAO);
	glGenBuffers(1, &skyVBO);

	GLuint EBO;
	glGenBuffers(1, &EBO);
	glBindVertexArray(skyVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyVBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	// transfer data
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesSky), verticesSky, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesX), indicesX, GL_STATIC_DRAW);

	// define size of data
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)(3 * sizeof(GLfloat)));
	//glEnableVertexAttribArray(3);
	//glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)(5 * sizeof(GLfloat)));


	// unfocus
	glBindVertexArray(0);
}

void createGround() {
	createTexture(textureGround, "Images/ground2.jpg");
	GLfloat y = -0.5f, z = groundHeight;

	GLfloat verticesGround[] = {
		// triangle 1   texture     normals 
		-groundWidth, y, -z,	512.0f,0.0f,  0.0f,1.0f,0.0f,
		groundWidth, y, -z,		512.0f,512.0f,  0.0f,1.0f,0.0f,
		-groundWidth, y, z,		0.0f,512.0f,  0.0f,1.0f,0.0f,
		groundWidth,  y, z,		0.0f,0.0f,  0.0f,1.0f,0.0f
	};

	GLuint indices[] = {
		0,1,2,
		2,3,1
	};

	glGenVertexArrays(1, &groundVAO);
	glGenBuffers(1, &groundVBO);

	GLuint EBO;
	glGenBuffers(1, &EBO);
	glBindVertexArray(groundVAO);
	glBindBuffer(GL_ARRAY_BUFFER, groundVBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	// transfer data
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesGround), verticesGround, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// define size of data
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)(3 * sizeof(GLfloat)));
	//glEnableVertexAttribArray(3);
	//glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)(5 * sizeof(GLfloat)));


	// unfocus
	glBindVertexArray(0);
}

void createGrass() {
	createTexture(textureGrass, "Images/grass1.jpg");
	GLfloat y = -0.4f, z = groundHeight - 50.0f, x = groundWidth/2;

	GLfloat verticesGrass[] = {
		// triangle 1   texture     normals 
		-x, y, -z,		25.0f,0.0f,  0.0f,1.0f,0.0f,
		x, y, -z,		25.0f,25.0f,  0.0f,1.0f,0.0f,
		-x, y, z,		0.0f,25.0f,  0.0f,1.0f,0.0f,
		x,  y, z,		0.0f,0.0f,  0.0f,1.0f,0.0f
	};

	GLuint indices[] = {
		0,1,2,
		2,3,1
	};

	glGenVertexArrays(1, &grassVAO);
	glGenBuffers(1, &grassVBO);

	GLuint EBO;
	glGenBuffers(1, &EBO);
	glBindVertexArray(grassVAO);
	glBindBuffer(GL_ARRAY_BUFFER, grassVBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	// transfer data
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesGrass), verticesGrass, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// define size of data
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)(3 * sizeof(GLfloat)));
	//glEnableVertexAttribArray(3);
	//glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)(5 * sizeof(GLfloat)));


	// unfocus
	glBindVertexArray(0);
}

void createBuilding()
{
	createTexture(textureBuilding[0], "Images/building1.jpg");
	createTexture(textureBuilding[1], "Images/building3.jpg");
	createTexture(textureBuilding[2], "Images/building4.png");
	createTexture(textureBuilding[3], "Images/building2.jpg");
	createTexture(textureBuilding[4], "Images/building4.jpg");

	GLfloat y1 = -0.5f, y2 = 3.0f, x = 1.0f, z = x;

	GLfloat verticesBuilding[] = {
		//floor		   texture     normals 
		-x, y1, -z,		1.0f,0.0f,  0.0f,-1.0f,0.0f,
		x, y1, -z,		0.0f,0.0f,  0.0f,-1.0f,0.0f,
		-x, y1, z,		1.0f,1.0f,  0.0f,-1.0f,0.0f,
		x,  y1, z,		0.0f,1.0f,  0.0f,-1.0f,0.0f,
		//ceiling		texture		normals
		-x, y2, -z,		1.0f,0.0f,  0.0f,1.0f,0.0f,
		x, y2, -z,		0.0f,0.0f,  0.0f,1.0f,0.0f,
		-x, y2, z,		1.0f,1.0f,  0.0f,1.0f,0.0f,
		x,  y2, z,		0.0f,1.0f,  0.0f,1.0f,0.0f,
		//right-wall	texture		normals
		x, y2, -z,		1.0f,0.0f,  1.0f,0.0f,0.0f,
		x, y2, z,		0.0f,0.0f,  1.0f,0.0f,0.0f,
		x, y1, -z,		1.0f,1.0f,  1.0f,0.0f,0.0f,
		x,  y1, z,		0.0f,1.0f,  1.0f,0.0f,0.0f,
		//left-wall		texture		normals
		-x, y2, z,		1.0f,0.0f,  -1.0f,0.0f,0.0f,
		-x, y2, -z,		0.0f,0.0f,  -1.0f,0.0f,0.0f,
		-x, y1, z,		1.0f,1.0f,  -1.0f,0.0f,0.0f,
		-x, y1, -z,		0.0f,1.0f,  -1.0f,0.0f,0.0f,
		//back			texture		normals
		-x, y2, -z,		1.0f,0.0f, 0.0f,0.0f,-1.0f,
		x, y2, -z,		0.0f,0.0f, 0.0f,0.0f,-1.0f,
		-x, y1, -z,		1.0f,1.0f, 0.0f,0.0f,-1.0f,
		x, y1, -z,		0.0f,1.0f, 0.0f,0.0f,-1.0f,

		//front			texture		normals
		-x, y2, z,		1.0f,0.0f,  0.0f,0.0f,1.0f,
		x, y2, z,		0.0f,0.0f,  0.0f,0.0f,1.0f,
		-x, y1, z,		1.0f,1.0f,  0.0f,0.0f,1.0f,
		x, y1, z,		0.0f,1.0f,  0.0f,0.0f,1.0f
	};

	GLuint indicesA[] = {
		0,1,2,
		2,3,1,

		4,5,6,
		6,7,5,

		8,9,10,
		10,11,9,

		12,13,14,
		14,15,13,

		18,16,17,
		18,19,17,

		20,21,22,
		22,23,21
	};

	generateBuildings();
	glGenBuffers(1, &instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * translations.size(), &translations[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glGenBuffers(1, &instanceVBO2);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * scalings.size(), &scalings[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	/*
	glGenBuffers(1, &instanceVBO2);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * scalings.size(), &scalings[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	*/	
	glGenVertexArrays(1, &buildingVAO);
	glGenBuffers(1, &buildingVBO);

	GLuint EBO;
	glGenBuffers(1, &EBO);
	glBindVertexArray(buildingVAO);
	glBindBuffer(GL_ARRAY_BUFFER, buildingVBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	// transfer data
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesBuilding), verticesBuilding, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesA), indicesA, GL_STATIC_DRAW);

	// define size of data
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)(3 * sizeof(GLfloat)));
	//glEnableVertexAttribArray(3);
	//glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)(5 * sizeof(GLfloat)));
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
	glVertexAttribDivisor(3, 1);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO2);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
	glVertexAttribDivisor(4, 1);

	// unfocus
	glBindVertexArray(0);

}

void createTexture(GLuint &texture, char* imageLocation)
{
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// Set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// Set texture wrapping to GL_REPEAT (usually basic wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int width2, height2;
	unsigned char* image = SOIL_load_image(imageLocation, &width2, &height2, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width2, height2, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void do_movement()
{
	// Camera controls
	GLfloat cameraSpeed = 1.0f;
	if (keys[GLFW_KEY_W])
		cameraPos += cameraFront * cameraSpeed;
	if (keys[GLFW_KEY_S])
		cameraPos -= cameraFront * cameraSpeed;
	if (keys[GLFW_KEY_A])
		cameraPos -= cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp));
	if (keys[GLFW_KEY_D])
		cameraPos += cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp));
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) // for resizing window
{
	glViewport(0, 0, width, height);
}

void generateBuildings()
{
	int x = 0, z = 0;
	bool xSign, ySign;
	//assign random translation vectors bounded in-between the park and the ground
	while (translations.size() < totalBuildings) {
		x = (std::rand() % 100);
		z = (std::rand() % 200);
		xSign = (std::rand() % 2) == 0;
		ySign = (std::rand() % 2) == 0;
		if((GLfloat)x > (groundWidth/2 + 5.0f) || (GLfloat)z > 153.0f){
			if (!xSign)
				x *= -1;
			if (!ySign)
				z *= -1;
			glm::vec3 translation((GLfloat)x, 0.0f,(GLfloat)z);
			translations.push_back(translation);
		}
	}
	//set specific scale values for type 1 building and so forth
		for(int i = 0; i<building1; i++){
			glm::vec3 scaling(1.0f, 1.0f, 1.0f);
			scalings.push_back(scaling);
		}
		for (int i = 0; i<building3; i++) {
			glm::vec3 scaling(4.0f, 10.0f, 2.0f);
			scalings.push_back(scaling);
		}
		for (int i = 0; i<building2; i++) {
			glm::vec3 scaling(3.0f, 6.5f, 1.5f);
			scalings.push_back(scaling);
		}
		for (int i = 0; i<building4; i++) {
			glm::vec3 scaling(2.5f, 5.7f, 2.0f);
			scalings.push_back(scaling);
		}
		for (int i = 0; i<building5; i++) {
			glm::vec3 scaling(6.0f, 4.0f, 3.0f);
			scalings.push_back(scaling);
		}
	
}