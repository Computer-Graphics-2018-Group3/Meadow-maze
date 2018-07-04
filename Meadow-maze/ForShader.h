#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <SDL2/SDL_opengl.h>

#include <iostream>

class ForShader
{
public:
	ForShader();
	~ForShader();

	static const GLchar* readShader(const char* filename);

	static GLuint makeProgram(const char* vertex, const char* fragment);
};

