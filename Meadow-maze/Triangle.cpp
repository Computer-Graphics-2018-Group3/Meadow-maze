#include "Triangle.h"
#include "ForShader.h"
#include "InputHandler.h"
#include "IL/il.h"
#include "IL/ilu.h"
#include "IL/ilut.h"
#include <SDL2/SDL_mixer.h>
#include <GLFW/glfw3.h>
#include <cmath>

#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif

Triangle::Triangle()
{
	//cout << "triangle construktor()" << endl;

	mouse_first_in = true;
}


Triangle::~Triangle()
{
	glDeleteProgram(shaders_animated_model);
}


void Triangle::init()
{
	// shader for animated model
	shaders_animated_model = ForShader::makeProgram("shader/animated_model.vert", "shader/animated_model.frag");
	model_man.loadModel("models/man/model.dae");
	model_man.initShaders(shaders_animated_model);
	position = glm::vec3(-4.0f, -0.60f, -4.0f);
	rotation = 0.0f;
}

void Triangle::update(Camera camera)
{
	//´°¿Ú´óÐ¡
	int SCR_WIDTH = 1280;
	int SCR_HEIGHT = 720;

	GLfloat current_frame = glfwGetTime();
	delta_time = (current_frame - last_frame);
	last_frame = current_frame;
	
	perspective_view = camera.GetViewMatrix();
	//perspective_view = glm::mat4();
	perspective_projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	//perspective_projection = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT);

	// model_man.update();
	// model 1 matrix (MAN)

    position.x = sin(glfwGetTime()) * 1.4f - 3.0f;
    position.z = cos(glfwGetTime()) * 1.4f - 3.9f;
    rotation = glm::degrees(glfwGetTime()) + 90.0f;

	matr_model_1 = glm::translate(glm::mat4(), position);
	matr_model_1 = glm::rotate(matr_model_1, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	matr_model_1 = glm::rotate(matr_model_1, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
	matr_model_1 = glm::scale(matr_model_1, glm::vec3(0.15f, 0.15f, 0.15f));
}

void Triangle::render(Camera camera, Shader* shader)
{
	if (shader == NULL) {
		glUseProgram(shaders_animated_model);

		glUniform3f(glGetUniformLocation(shaders_animated_model, "view_pos"), camera.Position.x, camera.Position.y, camera.Position.z);
		glUniform1f(glGetUniformLocation(shaders_animated_model, "material.shininess"), 32.0f);
		glUniform1f(glGetUniformLocation(shaders_animated_model, "material.transparency"), 1.0f);
		// Point Light 1
		glUniform3f(glGetUniformLocation(shaders_animated_model, "point_light.position"), camera.Position.x, camera.Position.y, camera.Position.z);

		glUniform3f(glGetUniformLocation(shaders_animated_model, "point_light.ambient"), 0.1f, 0.1f, 0.1f);
		glUniform3f(glGetUniformLocation(shaders_animated_model, "point_light.diffuse"), 1.0f, 1.0f, 1.0f);
		glUniform3f(glGetUniformLocation(shaders_animated_model, "point_light.specular"), 1.0f, 1.0f, 1.0f);

		glUniform1f(glGetUniformLocation(shaders_animated_model, "point_light.constant"), 1.0f);
		glUniform1f(glGetUniformLocation(shaders_animated_model, "point_light.linear"), 0.007);
		glUniform1f(glGetUniformLocation(shaders_animated_model, "point_light.quadratic"), 0.0002);

		//matr_model_1 = glm::mat4();
		//matr_model_1 = glm::rotate(matr_model_1, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		//matr_model_1 = glm::rotate(matr_model_1, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		//matr_model_1 = glm::rotate(matr_model_1, glm::radians(-camera.Yaw), glm::vec3(-camera.Position.x, camera.Position.y, camera.Position.z));
		//matr_model_1 = glm::scale(matr_model_1, glm::vec3(0.1f, 0.1f, 0.1f));
		//matr_model_1 = glm::translate(matr_model_1, glm::vec3(-5.0f, 2.0f, -5.0f)+glm::vec3(-camera.Position.x *10, camera.Position.z*5, camera.Position.y));

		MVP = perspective_projection * perspective_view * matr_model_1;
		//MVP = perspective_projection * matr_model_1;
		//MVP =  perspective_view * matr_model_1;
		glUniformMatrix4fv(glGetUniformLocation(shaders_animated_model, "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
		glUniformMatrix4fv(glGetUniformLocation(shaders_animated_model, "M_matrix"), 1, GL_FALSE, glm::value_ptr(matr_model_1));
		glm::mat4 matr_normals_cube = glm::mat4(glm::transpose(glm::inverse(matr_model_1)));
		glUniformMatrix4fv(glGetUniformLocation(shaders_animated_model, "normals_matrix"), 1, GL_FALSE, glm::value_ptr(matr_normals_cube));
	} else {
		shader->use();
		shader->setMat4("model", matr_model_1);
	}
	model_man.draw(shaders_animated_model);

	glUseProgram(0);
}

void Triangle::playSound()
{
	if (Mix_PlayingMusic() == 0)
		Mix_PlayMusic(music2, 1); // play next ( two time <- loop == 1 )

	model_man.playSound();
}

GLuint Triangle::loadImageToTexture(const char* image_path)
{

	ILuint ImageName; // The image name to return.
	ilGenImages(1, &ImageName); // Grab a new image name.
	ilBindImage(ImageName);
	if (!ilLoadImage((ILstring)image_path)) std::cout << "image NOT load " << std::endl;
	// we NEED RGB image (for not transparent)
	//ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE); //Convert image to RGBA with unsigned byte data type

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0, ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_TYPE), ilGetData());
	glGenerateMipmap(GL_TEXTURE_2D);

	ilDeleteImages(1, &ImageName);
	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}

GLuint Triangle::loadDDS(const char* image_path, int *w, int *h)
{
	// for use this method better way :
	// compress your image, save like .DDS ( with photoshop and nvidia utilite )
	// give address image to this method
	// method returns complete !! OpenGL texture for drawing on screen
	unsigned char header[124];

	FILE *fp = fopen(image_path, "rb");
	if (fp == NULL)
	{
		std::cout << "image not load \n";
		return 0;
	}

	char filecode[4];
	fread(filecode, 1, 4, fp);
	if (strncmp(filecode, "DDS ", 4) != 0) {
		std::cout << "return 0; \n";
		fclose(fp);
		return 0;
	}

	fread(&header, 124, 1, fp);

	unsigned int height = *(unsigned int*)&(header[8]);
	unsigned int width = *(unsigned int*)&(header[12]);
	unsigned int linear_size = *(unsigned int*)&(header[16]);
	unsigned int mipmap_count = *(unsigned int*)&(header[24]);
	unsigned int four_cc = *(unsigned int*)&(header[80]);  // formats

	if (w != nullptr) *w = width;
	if (h != nullptr) *h = height;

	std::cout << "image: " << image_path << " mipmap_count = " << mipmap_count << std::endl;


	unsigned char *buffer;
	unsigned int buff_size = mipmap_count > 1 ? linear_size * 2 : linear_size;
	buffer = (unsigned char*)malloc(buff_size * sizeof(unsigned char));
	fread(buffer, 1, buff_size, fp);
	
	fclose(fp);

#define FOURCC_DXT1 0x31545844 // "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844 // "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844 // "DXT5" in ASCII

	unsigned int components = (four_cc == FOURCC_DXT1) ? 3 : 4;;
	unsigned int format;
	if (four_cc == FOURCC_DXT1)
		format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
	else if (four_cc == FOURCC_DXT3)
		format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
	else if (four_cc == FOURCC_DXT5)
		format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;

#undef FOURCC_DXT1
#undef FOURCC_DXT3
#undef FOURCC_DXT5

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	if (mipmap_count == 1) // if we have only 1 image 0 level ( not mipmap )
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);	// highest resolution
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);	// lowest resolution
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else // we have mipmap
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);	//  highest resolution
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmap_count);	// lowest resolution
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}

	unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;;
	unsigned int offset = 0;

	// load mipmap. if we have not mipmap mipmap_count = 1 and be loaded 1 image (0 level)
	for (unsigned int level = 0; level < mipmap_count && (width || height); level++)
	{
		unsigned int size = ((width + 3) / 4)*((height + 3) / 4)*blockSize;
		glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height, 0, size, buffer + offset);

		offset += size;
		width /= 2;
		height /= 2;
	}
	free(buffer);

	return textureID;
}