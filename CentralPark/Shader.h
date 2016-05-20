#include <stdio.h>
#include <GL/glew.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
class Shader
{
public:
	GLuint program;

	Shader(const GLchar* vertexPath, const GLchar* fragmentPath);

	void use();
};
