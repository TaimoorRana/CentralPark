// Other headers
#include "stdafx.h"

#include <Windows.h>
#include <iostream>
#include <string>
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

using namespace std;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void createGround();
void createBuilding();
void createTexture(GLuint &texture, char* imageLocation);
void do_movement();
void createAllBuildingTextures();
void createBuildingModelMatrices();
GLuint loadCubemap(vector<const GLchar*> faces);
void generateSkybox();
bool test1xaxis(int v4, int v3, int xf);
bool test2zaxis(int v4, int v1, int zf);
bool isInsideOccupiedAreaTest(int v4x, int v4z, int v3x, int v1z, int xf, int zf);
void getUserInput();
void welcomeDisplay();
bool intelliConsoleResponse(int numberOfBuilding);
int getIntegerFromInput(string s);
void initialiseWindow();
void printProgressReport(int i);
void mouse_position_callback(GLFWwindow* window, double xPos, double yPos);
void createPark();

// Window dimensions
const GLuint WIDTH = 1600, HEIGHT = 1200;
GLFWwindow* window;
Shader *groundShader;
int buildingProgress; // for starting flow
//ground
GLuint groundVAO, groundVBO;
GLuint textureGround;
GLfloat groundWidth = 1000.0f;

//park
GLuint parkVAO, parkVBO;
GLuint texturePark;
GLfloat parkWidth = 100.0f;

//buildings
GLuint buildingVAO, buildingVBO, instanceVBO;
std::vector<GLuint> textureBuilding;
int totalBuildings = 10000;
std::vector<glm::vec3> buldingTranslations;
std::vector<char*> buildingImagesLocations;
std::vector<glm::mat4> buildingModelMatrices; // used for scaling buildings
GLfloat highestScaleValue = 10.0f; // used for scaling buildings


//camera
glm::vec3 cameraPos(0.0f, 3.0f, 0.0f), cameraFront(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp(glm::normalize(glm::cross( glm::vec3(1,0,0), cameraFront)));
GLfloat yaw = -90.0f;	// Yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right (due to how Eular angles work) so we initially rotate a bit to the left.
GLfloat pitch = 0.0f;
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];

//skybox
Shader * skyboxShader;
GLuint skyboxVAO;
GLuint cubemapTexture;

// mouse
bool firstMouse = true;

int main()
{
	//getUserInput();
	initialiseWindow();

	// GAME LOOP START HERE
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (GLfloat)WIDTH / (GLfloat)HEIGHT, 1.0f, 400.0f);
	while (!glfwWindowShouldClose(window))
	{
		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		do_movement();
		
		// Render
		// Clear the colorbuffer
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// Draw skybox first
		glEnable(GL_DEPTH_CLAMP);
		glDepthMask(GL_FALSE);// Remember to turn depth writing off
		skyboxShader->use();
		glm::mat4 view = glm::mat4(glm::mat3(glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp)));	// Remove any translation component of the view matrix
		
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
		// need a better for loop 
		glBindVertexArray(buildingVAO);
		int BuildingDivisionByTexture = totalBuildings / textureBuilding.size();
		int buildingToDraw = BuildingDivisionByTexture;
		for (int i = 0; i < textureBuilding.size(); i++) {
			glBindTexture(GL_TEXTURE_2D, textureBuilding[i]);
			glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, buildingToDraw);
			buildingToDraw += BuildingDivisionByTexture;
		}
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, texturePark);

		// create ground
		glBindVertexArray(parkVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
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

	GLfloat textureSize = 200;
	GLfloat verticesGround[] = {
		// triangle 1   texture									normals 
		-groundWidth, y, -z,		0.0f, 0.0f,					0.0f,1.0f,0.0f,
		groundWidth, y, -z,		    textureSize,0.0f,			0.0f,1.0f,0.0f,
		-groundWidth, y, z,			0.0f,textureSize,			0.0f,1.0f,0.0f,
		groundWidth,  y, z,			textureSize,textureSize,	0.0f,1.0f,0.0f
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

void createPark() {
	createTexture(texturePark, "Images/park.jpg");
	GLfloat y = -0.5f, z = parkWidth;

	GLfloat textureSize = 200;
	GLfloat verticesPark[] = {
		// triangle 1						texture						normals 
		-parkWidth, y, -parkWidth,			0.0f, 0.0f,					0.0f,1.0f,0.0f,
		parkWidth, y, -parkWidth,		    textureSize,0.0f,			0.0f,1.0f,0.0f,
		-parkWidth, y, parkWidth,			0.0f,textureSize,			0.0f,1.0f,0.0f,
		parkWidth,  y, parkWidth,			textureSize,textureSize,	0.0f,1.0f,0.0f
	};

	GLuint indices[] = {
		0,1,2,
		2,3,1
	};

	glGenVertexArrays(1, &parkVAO);
	glGenBuffers(1, &parkVBO);

	GLuint EBO;
	glGenBuffers(1, &EBO);
	glBindVertexArray(parkVAO);
	glBindBuffer(GL_ARRAY_BUFFER, parkVBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	// transfer data
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesPark), verticesPark, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// define size of data
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)(3 * sizeof(GLfloat)));
	//glEnableVertexAttribArray(3);
	//glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)(5 * sizeof(GLfloat)));

	glm::mat4 model(1.0f);
	model = glm::translate(model, glm::vec3(0.0, 0.1, 0.0));
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

	

	// save the translations in a VBO
	


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
	GLfloat cameraSpeed = 0.05f;
	if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
		cameraPos += glm::vec3(cameraFront.x * cameraSpeed, 0, cameraFront.z * cameraSpeed);
	if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
		cameraPos -= glm::vec3(cameraFront.x * cameraSpeed, 0, cameraFront.z * cameraSpeed);
	if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT]) {

		GLfloat xoffset = -1.0f;

		GLfloat sensitivity = 0.05;	// Change this value to your liking
		xoffset *= sensitivity;

		yaw += xoffset;

		glm::vec3 front;
		front.x = cos(glm::radians(yaw));
		front.z = sin(glm::radians(yaw));
		cameraFront = glm::normalize(front);
	}
	if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) {
		GLfloat xoffset = 1.0f;

		GLfloat sensitivity = 0.05;	// Change this value to your liking
		xoffset *= sensitivity;

		yaw += xoffset;

		glm::vec3 front;
		front.x = cos(glm::radians(yaw));
		front.z = sin(glm::radians(yaw));
		cameraFront = glm::normalize(front);
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
	//buildingImagesLocations.push_back("Images/building3.jpg");
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
	
	//for translation
	std::random_device rd; // dsobtain a random number from hardware
	std::mt19937 eng(rd()); // seed the generator
	std::uniform_int_distribution<> distr(0, groundWidth - 1);

	// for scaling
	std::random_device rd2; // dsobtain a random number from hardware
	std::mt19937 eng2(rd2()); // seed the generator
	std::uniform_int_distribution<> distr2(highestScaleValue - 6, highestScaleValue); // define the range // -6 was added to get rid of very slim buildings

	int parkPadding = 10;
	// while all building positions have not been defined
	while (buildingModelMatrices.size() < totalBuildings) {

		// generate x, z values
		int x = distr(eng), z = distr(eng);



		// make sure the values are on the ground surface
		if ((x < groundWidth && (x > parkWidth || (x < parkWidth + parkPadding && z > parkWidth + parkPadding))) &&
			(z < groundWidth && (z > parkWidth || (z < parkWidth + parkPadding && x > parkWidth + parkPadding)))) {
			// randomly assign negative values to x and z
			bool xSign = (std::rand() % 2) == 0;
			bool ySign = (std::rand() % 2) == 0;
			if (!xSign)
				x *= -1;
			if (!ySign)
				z *= -1;

			glm::vec3 translation(x, 0, z);
			glm::mat4 model;
			model = glm::translate(model, translation);
			model = glm::scale(model, glm::vec3(distr2(eng2), distr2(eng2), distr2(eng2)));

			buildingModelMatrices.push_back(model);
		}
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


// ----------------------------------------------------------------------------
//   STARTING SCREEN STUFF                                                    +
// ----------------------------------------------------------------------------

/*	
	function that welcome the user and request an input from him/her
	function handle all kind of input, or so I thought
*/
void getUserInput() {  
	welcomeDisplay();
	bool userContinueToInput = true;
	string currentInput;
	int numberOfBuildingToGenerate;
	cout << "Hello!\nWelcome to our mini city buildings mass builder,\nJust enter a number, any big number (ok maybe not 1 billion).\n" << endl;
	system("PAUSE");
	while (userContinueToInput) {
		system("CLS"); // clear console screen
		welcomeDisplay();
		cout << "Choose your number yet?" << endl;
		cin >> currentInput;
		numberOfBuildingToGenerate = getIntegerFromInput(currentInput);
		userContinueToInput = intelliConsoleResponse(numberOfBuildingToGenerate);	
		system("PAUSE");
	}
	totalBuildings = numberOfBuildingToGenerate;
	system("CLS"); 
	welcomeDisplay();
	cout << "Let me call my children the threads:" << endl;
}

/*	
	handle string and integer input to avoid crashing
	iterate through the string variable and extract the integer, then convert into int data type
	else if there's no integer return code -1234567890
*/
int getIntegerFromInput(string s) {
	string intExtract;
	for (unsigned int i = 0; i < s.size(); i++) {
		if (isdigit(s[i]))
		{
			intExtract += s[i];
		}
	}
	if (!intExtract.empty()) {
		return stoi(intExtract);
	}
	else return -1234567890;
}

/* 
	just the console trying to be a smart cookie, analysizing the integer to generate the number of buildings
*/
bool intelliConsoleResponse(int numberOfBuilding) {
	if (numberOfBuilding == -1234567890) {
		cout << "Letters, letters, only letters and no number! Don't try to foul me.\nCome on, feed me a number!\n" << endl;
		return true;
	}
	else if (numberOfBuilding == 0) {
		cout << "Zero? You don't trust in my capability? I'm an Intel i7! Challenge me!\n" << endl;
		return true;
	}
	else if (numberOfBuilding > 0 && numberOfBuilding < 1000) {
		cout << "I thought we are building an entire city, not just a village!\nAre you sure you want only " << numberOfBuilding << " (y/n) ?" << endl;
		string response;
		while (response.empty())
			cin >> response;
		if (response == "y" || response == "Y") {
			cout << "Ok fine. I will do it, just for you!\n" << endl;
			return false;
		}
		else if (response == "n" || response == "N") {
			cout << "I thought so.\n" << endl;
			return true;
		}
		else {
			cout << "I did not understand, say again?\n" << endl;
			response.clear();
		}
	}
	else if (numberOfBuilding > 999) {
		cout << "That looks a pretty good number.\n" << endl;
		return false;
	}
	else {
		cout << "Is that even a number?\n" << endl;
		return true;
	}
}

/*
	show number of building being generated
*/
void printProgressReport(int i) {
	cout << "\rBuilt the " << i << "th building... working hard";
}

/*
	display ascii art, from  http://www.ascii-code.com/ascii-art/buildings-and-places/cities.php
*/
void welcomeDisplay() {
	cout << " " << endl;
	cout << " P r o c e d u r a l l y   G e n e r a t e d   M a n h a t t a n ' s    C e n t r a l    P a r k" << endl;
	cout << "    -------------------------------------------------------------------------------------------" << endl;
	cout << "                             ________            _______" << endl;
	cout << "                    /\\ \\ \\ \\/_______/     ______/\\      \\  /\\ \\/ /\\ \\/ /\\  \\_____________" << endl;
	cout << "                   /\\ \\ \\ \\/______ /     /\\    /:\\\\      \\ ::\\  /::\\  /::\\ /____  ____ __" << endl;
	cout << "                  /\\ \\ \\ \\/_______/     /:\\\\  /:\\:\\\\______\\::/  \\::/  \\::///   / /   //" << endl;
	cout << "                 /\\ \\ \\ \\/_______/    _/____\\/:\\:\\:/_____ / / /\\ \\/ /\\ \\///___/ /___//___" << endl;
	cout << "           _____/___ \\ \\/_______/    /\\::::::\\\\:\\:/_____ / \\ /::\\  /::\\ /____  ____  ____" << endl;
	cout << "                    \\ \\/_______/    /:\\\\::::::\\\\:/_____ /   \\\\::/  \\::///   / /   / /   /" << endl;
	cout << "                     \\/_______/    /:\\:\\\\______\\/______/_____\\\\/ /\\ \\///___/ /___/ /_____" << endl;
	cout << "           \\          \\______/    /:\\:\\:/_____:/\\      \\ ___ /  /::\\ /____  ____  _/\\::::" << endl;
	cout << "           \\\\__________\\____/    /:\\:\\:/_____:/:\\\\      \\__ /_______/____/_/___/_ /  \\:::" << endl;
	cout << "           //__________/___/   _/____:/_____:/:\\:\\\\______\\ /                     /\\  /\\::" << endl;
	cout << "           ///\\          \\/   /\\ .----.\\___:/:\\:\\:/_____ // \\                   /  \\/  \\:" << endl;
	cout << "           ///\\\\          \\  /::\\\\ \\_\\ \\\\_:/:\\:\\:/_____ //:\\ \\                 /\\  /\\  /\\" << endl;
	cout << "           //:/\\\\          \\//\\::\\\\ \\ \\ \\\\/:\\:\\:/_____ //:::\\ \\               /  \\/  \\/+/" << endl;
	cout << "           /:/:/\\\\_________/:\\/:::\\`----' \\\\:\\:/_____ //o:/\\:\\ \\_____________/\\  /\\  / /" << endl;
	cout << "           :/:/://________//\\::/\\::\\_______\\\\:/_____ ///\\_\\ \\:\\/____________/  \\/  \\/+/\\" << endl;	
	cout << "           /:/:///_/_/_/_/:\\/::\\ \\:/__  __ /:/_____ ///\\//\\\\/:/ _____  ____/\\  /\\  / /  \\" << endl;
	cout << "           :/:///_/_/_/_//\\::/\\:\\///_/ /_//:/______/_/ :~\\/::/ /____/ /___/  \\/  \\/+/\\  /" << endl;
	cout << "           /:///_/_/_/_/:\\/::\\ \\:/__  __ /:/____/\\  / \\\\:\\/:/ _____  ____/\\  /\\  / /  \\/" << endl;
	cout << "           :///_/_/_/_//\\::/\\:\\///_/ /_//:/____/\\:\\____\\\\::/ /____/ /___/  \\/  \\/+/\\  /\\" << endl;
	cout << "           ///_/_/_/_/:\\/::\\ \\:/__  __ /:/____/\\:\\/____/\\\\/____________/\\  /\\  / /  \\/  \\" << endl;
	cout << "           //_/_/_/_//\\::/\\:\\///_/ /_//::::::/\\:\\/____/  /----/----/--/  \\/  \\/+/\\  /\\  /" << endl;
	cout << "           /_/_/_/_/:\\/::\\ \\:/__  __ /\\:::::/\\:\\/____/ \\/____/____/__/\\  /\\  / /  \\/  \\/_" << endl;
	cout << "              ----------------------------------------------------------------------" << endl;
	cout << "               ascii art from ascii-code.com                          An OpenGL 3.3" << endl;
	cout << "=================================================================================================" << endl;
	cout << " CONTROL : G - generate city   N - generate new buildings (to add all the control here) \n" << endl;
	cout << "_________________________________________________________________________________________________" << endl;

}

// ----------------------------------------------------------------------------
//   INITIALIZATION OF GLFW & CIE STUFF                                       +
// ----------------------------------------------------------------------------

void initialiseWindow() {
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	// Create a GLFWwindow object that we can use for GLFW's functions
	window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		//return -1;
	}
	glfwMakeContextCurrent(window);
	// Set the required callback functions
	glfwSetCursorPosCallback(window, mouse_position_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); // for window resize
																	   // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		//return -1;
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
	createBuilding();//VAO VBO 1
	createPark();
 	

	glEnable(GL_DEPTH_TEST);
}

void mouse_position_callback(GLFWwindow * window, double xPos, double yPos)
{
	
	/* Taken from learnopengl.com*/
	if (firstMouse)
	{
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}

	GLfloat xoffset = xPos - lastX;
	GLfloat yoffset = lastY - yPos; // Reversed since y-coordinates go from bottom to left
	lastX = xPos;
	lastY = yPos;

	GLfloat sensitivity = 0.05;	// Change this value to your liking
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
	/* Taken from learnopengl.com*/
	
}