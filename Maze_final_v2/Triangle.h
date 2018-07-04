#pragma once

#include <iostream>
#include "maze/camera.h"
#include "maze/shader.h"
#include "Model.h"
#include "Vector2D.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SDL2/SDL_mixer.h>

class Triangle
{
public:
	Triangle();
	~Triangle();

	GLfloat speed = 0.02f;
	//for camera
	GLfloat delta_time = 0.0f;
	GLfloat last_frame = 0.0f;
	GLfloat last_x;
	GLfloat last_y;
	Vector2D mouse_position;
	GLboolean mouse_first_in;

	GLuint shaders_animated_model;
	// Shader* animationShader;
	Model model_man;
	Model model_astroboy;

	// music
	Mix_Music* music1 = nullptr;
	Mix_Music* music2 = nullptr;

	glm::mat4 MVP;
	glm::mat4 perspective_view;
	glm::mat4 perspective_projection;
	glm::mat4 matr_model_1;
	glm::mat4 matr_model_2;

	glm::vec3 position;
	float rotation;

	// text
	glm::mat4 text_matrix_2D; // orthographic projection
	glm::mat4 text_matrix_3D_model_1; // perspective  projection
	glm::mat4 text_matrix_3D_model_2; // perspective  projection

	void init();
	void update(Camera camera);
	void render(Camera camera, Shader* shader = NULL);
	void playSound();

	static GLuint loadDDS(const char* image_path, int* w = nullptr, int* h = nullptr);
	static GLuint loadImageToTexture(const char* image_path);

};

