#include <stdio.h>
#include <GL/glew.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
using namespace std;
class Shader
{
public:
	// Where vertex and fragment shader are linked
	GLuint program;

	Shader(const GLchar* vertexPath, const GLchar* fragmentPath);

	void createProgram(string vertexCode, string fragmentCode);
	// use the program created by the constructor
	void use();
};
