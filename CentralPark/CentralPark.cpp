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
#include <random>


// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void createGround();
void createBuilding();
void createTexture(GLuint &texture, char* imageLocation);
void do_movement();
void generateBuildings();
void createAllBuildingTextures();
void createBuildingModelMatrices();
GLuint loadCubemap(vector<const GLchar*> faces);
void generateSkybox();
bool test1xaxis(int v4, int v3, int xf);
bool test2zaxis(int v4, int v1, int zf);
bool isInsideOccupiedAreaTest(int v4x, int v4z, int v3x, int v1z, int xf, int zf);


// Window dimensions
const GLuint WIDTH = 1600, HEIGHT = 1200;
Shader *groundShader;
//ground
GLuint groundVAO, groundVBO;
GLuint textureGround;
GLfloat groundWidth = 1000.0f;
//buildings
GLuint buildingVAO, buildingVBO, instanceVBO;
std::vector<GLuint> textureBuilding;
int totalBuildings = 5000;
std::vector<glm::vec3> buldingTranslations;
std::vector<char*> buildingImagesLocations;
std::vector<glm::mat4> buildingModelMatrices; // used for scaling buildings
GLfloat highestScaleValue = 10.0f; // used for scaling buildings

// street 
GLfloat streetWidth = 5.0f;

//camera
glm::vec3 cameraPos(0.0f, 3.0f, 0.0f), cameraFront(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp(glm::normalize(glm::cross( glm::vec3(1,0,0), cameraFront)));
bool keys[1024];

//skybox
Shader * skyboxShader;
GLuint skyboxVAO;
GLuint cubemapTexture;

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

	

	// initialize shaders
	groundShader = new Shader("TextFiles/vertex.shader", "TextFiles/fragment.shader");
	skyboxShader = new Shader("TextFiles/skyBoxVertex.shader", "TextFiles/skyBoxFragment.shader");
	generateSkybox();
	createAllBuildingTextures();
	createBuildingModelMatrices();
	createGround();
	createBuilding();


	std::random_device rd; // obtain a random number from hardware
	std::mt19937 eng(rd()); // seed the generator
	std::uniform_int_distribution<> distr(0, 4); // define the range

	glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(window))
	{
		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		do_movement();
		//Sleep(250);
		generateBuildings();
		// Render
		// Clear the colorbuffer
		glClearColor(0.0f, 0.3f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// Draw skybox first
		glDepthMask(GL_FALSE);// Remember to turn depth writing off
		skyboxShader->use();
		glm::mat4 view = glm::mat4(glm::mat3(glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp)));	// Remove any translation component of the view matrix
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (GLfloat)WIDTH / (GLfloat)HEIGHT, 1.0f, 100.0f);
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader->program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader->program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		// skybox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(skyboxShader->program, "skybox"), 0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthMask(GL_TRUE);






		groundShader->use(); 

		//glm::mat4 view2;
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

		glm::mat4  model(1.0f);
		projection = glm::perspective(glm::radians(45.0f), (GLfloat)WIDTH / (GLfloat)HEIGHT, 1.0f, 200.0f);

		GLint modelLoc = glGetUniformLocation(groundShader->program, "model");
		GLint viewLoc = glGetUniformLocation(groundShader->program, "view");
		GLint projLoc = glGetUniformLocation(groundShader->program, "projection");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glBindTexture(GL_TEXTURE_2D, textureGround);

		// create ground
		glBindVertexArray(groundVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// create buildings

		glBindTexture(GL_TEXTURE_2D, textureBuilding[3]);
		glBindVertexArray(buildingVAO);
		glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0,totalBuildings);
		
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

void createGround() {
	createTexture(textureGround, "Images/ground.jpg");
	GLfloat y = -0.5f, z = groundWidth;

	GLfloat verticesGround[] = {
		// triangle 1   texture     normals 
		-groundWidth, y, -z,	100.0f,0.0f,  0.0f,1.0f,0.0f,
		groundWidth, y, -z,		100.0f,100.0f,  0.0f,1.0f,0.0f,
		-groundWidth, y, z,		0.0f,100.0f,  0.0f,1.0f,0.0f,
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

	glm::mat4 model(1.0f);
	//model = glm::scale(model, glm::vec3(highestScaleValue));
	GLuint modelMatrixVBO;
	glGenBuffers(1, &modelMatrixVBO);
	glBindBuffer(GL_ARRAY_BUFFER, modelMatrixVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4), &model, GL_STATIC_DRAW);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (GLvoid*)0);
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (GLvoid*) sizeof(glm::vec4));
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (GLvoid*)(2 * sizeof(glm::vec4)));
	glEnableVertexAttribArray(7);
	glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (GLvoid*)(3 * sizeof(glm::vec4)));


	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);
	glVertexAttribDivisor(7, 1);

	// unfocus
	glBindVertexArray(0);
}

void createBuilding()
{
	
	GLfloat y1 = -0.5f, y2 = 3.0f, x = 1.0f, z = x;
	GLfloat textureSize = 6.0f;

	GLfloat verticesBuilding[] = {
		
		//floor		   texture                       normals 
		-x, y1, -z,		textureSize,0.0f,			 0.0f,-1.0f,0.0f,
		x, y1, -z,		0.0f,0.0f,					 0.0f,-1.0f,0.0f,
		-x, y1, z,		textureSize,textureSize,	 0.0f,-1.0f,0.0f,
		x,  y1, z,		0.0f,1.0f,                   0.0f,-1.0f,0.0f,
		//ceiling		texture		normals
		-x, y2, -z,		textureSize,0.0f,			 0.0f,1.0f,0.0f,
		x, y2, -z,		0.0f,0.0f,					 0.0f,1.0f,0.0f,
		-x, y2, z,		textureSize,textureSize,     0.0f,1.0f,0.0f,
		x,  y2, z,		0.0f,textureSize,            0.0f,1.0f,0.0f,
		//right-wall	texture		normals
		x, y2, -z,		textureSize,0.0f,			 1.0f,0.0f,0.0f,
		x, y2, z,		0.0f,0.0f,					 1.0f,0.0f,0.0f,
		x, y1, -z,		textureSize,textureSize,     1.0f,0.0f,0.0f,
		x,  y1, z,		0.0f,textureSize,            1.0f,0.0f,0.0f,
		//left-wall		texture		normals
		-x, y2, z,		textureSize,0.0f,            -1.0f,0.0f,0.0f,
		-x, y2, -z,		0.0f,0.0f,					 -1.0f,0.0f,0.0f,
		-x, y1, z,		textureSize,textureSize,     -1.0f,0.0f,0.0f,
		-x, y1, -z,		0.0f,textureSize,			 -1.0f,0.0f,0.0f,
		//back			texture		normals
		-x, y2, -z,		textureSize,0.0f,			 0.0f,0.0f,-1.0f,
		x, y2, -z,		0.0f,0.0f,					 0.0f,0.0f,-1.0f,
		-x, y1, -z,		textureSize,textureSize,	 0.0f,0.0f,-1.0f,
		x, y1, -z,		0.0f,textureSize,			 0.0f,0.0f,-1.0f,
		                
		//front			texture		normals
		-x, y2, z,		textureSize,0.0f,			 0.0f,0.0f,1.0f,
		x, y2, z,		0.0f,0.0f,					 0.0f,0.0f,1.0f,
		-x, y1, z,		textureSize,textureSize,     0.0f,0.0f,1.0f,
		x, y1, z,		0.0f,textureSize,			 0.0f,0.0f,1.0f
	};

	GLuint indices[] = {
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

	// calculate building translations
	generateBuildings();

	// save the translations in a VBO
	glGenBuffers(1, &instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * buldingTranslations.size(), &buldingTranslations[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	// define building VAO, VBO, EBO
	glGenVertexArrays(1, &buildingVAO);
	glGenBuffers(1, &buildingVBO);

	GLuint EBO;
	glGenBuffers(1, &EBO);
	glBindVertexArray(buildingVAO);
	glBindBuffer(GL_ARRAY_BUFFER, buildingVBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	// transfer data
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesBuilding), verticesBuilding, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// define position data
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)0);
	// define Texture position data
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)(3 * sizeof(GLfloat)));

	// define normal position data
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)(5 * sizeof(GLfloat)));

	// bind the translation buffer
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3,3,GL_FLOAT,GL_FALSE, 3 * sizeof(GLfloat),0);
	glVertexAttribDivisor(3, 1); // tell opengl that this is instanced data
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint modelMatrixVBO;
	glGenBuffers(1, &modelMatrixVBO);
	glBindBuffer(GL_ARRAY_BUFFER, modelMatrixVBO);
	glBufferData(GL_ARRAY_BUFFER, totalBuildings * sizeof(glm::mat4), &buildingModelMatrices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (GLvoid*)0);
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (GLvoid*) sizeof(glm::vec4));
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (GLvoid*)(2 * sizeof(glm::vec4)));
	glEnableVertexAttribArray(7);
	glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (GLvoid*)(3 * sizeof(glm::vec4)));


	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);
	glVertexAttribDivisor(7, 1);
	// unfocus
	glBindVertexArray(0);

}

/*
	creates textures
	-texture:       address where to store the final texture
	-imageLocation: image file location 
*/

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

/*
	changes camera positions
*/
void do_movement()
{
	// Camera controls
	GLfloat cameraSpeed = 0.01f;
	if (keys[GLFW_KEY_W])
		cameraPos += glm::vec3(cameraFront.x * cameraSpeed, 0, cameraFront.z * cameraSpeed);
	if (keys[GLFW_KEY_S])
		cameraPos -= glm::vec3(cameraFront.x * cameraSpeed, 0, cameraFront.z * cameraSpeed);
	if (keys[GLFW_KEY_A])
		cameraPos -= cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp));
	if (keys[GLFW_KEY_D])
		cameraPos += cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp));
}

/*
generate bulding positions
*/
void generateBuildings()
{

	// while all building positions have not been defined
	while (buldingTranslations.size() < totalBuildings) {

		// generate x, z values
		int x = (std::rand() % 100), z = (std::rand() % 100);

		// randomly assign negative values to x and z
		bool xSign = (std::rand() % 2) == 0;
		bool ySign = (std::rand() % 2) == 0;

		// make sure the values are on the ground surface
		if (x < groundWidth && x >= streetWidth && z < groundWidth && z >= streetWidth  ) {
			if (!xSign)
				x *= -1;
			if (!ySign)
				z *= -1;

			glm::vec3 translation(x, 0, z);
			buldingTranslations.push_back(translation);
		}
	}
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height) // for resizing window
{
	glViewport(0, 0, width, height);
}

/*
generate all buldings textures
*/
void createAllBuildingTextures() {
	buildingImagesLocations.push_back("Images/building1.jpg");
	buildingImagesLocations.push_back("Images/building2.jpg");
	buildingImagesLocations.push_back("Images/building3.jpg");
	buildingImagesLocations.push_back("Images/building4.jpg");
	buildingImagesLocations.push_back("Images/building5.jpg");
	for (int i = 0; i < buildingImagesLocations.size(); i++) {
		GLuint texture;
		glGenTextures(1, &texture);
		textureBuilding.push_back(texture);
		createTexture(textureBuilding[i], buildingImagesLocations[i]);
	}
}

/*
generate all buldings models
*/
void createBuildingModelMatrices() {
	std::random_device rd; // obtain a random number from hardware
	std::mt19937 eng(rd()); // seed the generator
	std::uniform_int_distribution<> distr(1, highestScaleValue); // define the range
	for (int i = 0; i < totalBuildings; i++) {
		glm::mat4 model;
		
		model = glm::scale(model, glm::vec3(distr(eng), distr(eng), distr(eng)));
		//std::cout
		buildingModelMatrices.push_back(model);
	}
}

/*
generates cubemap
*/
GLuint loadCubemap(vector<const GLchar*> faces)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);

	int width, height;
	unsigned char* image;

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for (GLuint i = 0; i < faces.size(); i++)
	{
		image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
			GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image
		);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return textureID;
}

/*
generates skybox
*/
void generateSkybox() {
	//skybox
	vector<const GLchar*> faces;
	faces.push_back("Images/right.jpg");
	faces.push_back("Images/left.jpg");
	faces.push_back("Images/top.jpg");
	faces.push_back("Images/bottom.jpg");
	faces.push_back("Images/back.jpg");
	faces.push_back("Images/front.jpg");
	cubemapTexture = loadCubemap(faces);


	GLfloat skyboxVertices[] = {
		// Positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};

	// Setup skybox VAO
	GLuint skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glBindVertexArray(0);
}

// ----------------------------------------------------------------------------
//   COLLISION DETECTION STUFF -  DURING BUILDING GENERATION                  +
// ----------------------------------------------------------------------------
// to do:
// 1) detect park function
// 2) test boundary function : use after testing if point is not in the area from test isInsideOccupiedAreaTest



/*	test if the the "foreign" point (randomly generated location point) is 
	in an area already occupied by taking 2 tests : x-axis and z-axis. 
	function takes in vertice position and the foreign position denoted with 'f'
*/
bool isInsideOccupiedAreaTest(int v4x, int v4z, int v3x, int v1z, int xf, int zf) {
	if (test1xaxis(v4x, v3x, xf) && test2zaxis(v4z, v1z, zf)) {
		cout << "point is inside  or on a vertice" << endl;  // to be removed once full implementation of collision detection is completed
		return true;
	}
	else {
		cout << "point is outside" << endl; // to be removed once full implementation of collision detection is completed
		return false;
	}
}

/*	first test if the "foreign" point (randomly generated location point) is 
	in an area already occupied by an existing building on the x-axis
*/
bool test1xaxis(int v4, int v3, int xf) {
	if (v4 <= xf && v3 >= xf) {
		cout << "point is between x4 and x3 or on a vertice" << endl;  // to be removed once full implementation of collision detection is completed
		return true;
	}
	else return false;
}

/*	second test if the "foreign" point (randomly generated location point) is
in an area already occupied by an existing building on the z-axis
*/
bool test2zaxis(int v4, int v1, int zf) {
	if (v4 <= zf && v1 >= zf) { 
		cout << "point is between z4 and z1  or on a vertice" << endl; // to be removed once full implementation of collision detection is completed
		return true;
	}
	else return false;
}

// ----------------------------------------------------------------------------
//   COLLISION DETECTION STUFF -  CAMERA MOVEMENT                             +
// ----------------------------------------------------------------------------
// to do:
// 1) test up, down, left, right to get closer object, set into a variable as boundary limit for -+x and -+z -> need 4 variables
//		this would avoid to constantly iterate throught the collection all the time
//		may use insertion sorting algorithm or binary search