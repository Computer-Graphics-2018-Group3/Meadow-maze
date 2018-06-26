// include glad, GLFW
#include <glad/glad.h>
#include <GL/glfw3.h>

// include our own files
#include "learnOpenGL\Camera.h"
#include "learnOpenGL\shader.h"
#include "learnOpenGL\stb_image.h"

// include glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// include C++ library
#include <iostream>

// function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
//void renderScene(Shader wallShader, unsigned int wallVAO, float clipPlane[4]);
void renderScene(Shader wallShader, float clipPlane[4]);
unsigned int loadTexture(char const * path);

unsigned int initializeReflectionFBO();
unsigned int initializeRefractionFBO();

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 900;

// camera
//Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;    // time between current frame and last frame
float lastFrame = 0.0f;

unsigned int reflectionColorBuffer;
unsigned int refractionColorBuffer;

float reflect_plane[4] = { 0, 1, 0, 0 };
float refract_plane[4] = { 0, -1, 0, 0 };
//float plane[4] = { 0, -1, 0, 100000 };
float plane[4] = { 0, -1, 0, 0 };
unsigned int wallVAO, floorVAO;
unsigned int texture1, texture2;

unsigned int DuDvTexture;
//unsigned int dudvMap;

unsigned int normalTexture;
//unsigned int normalMap;

float wave_speed = 0.0001f;
//float wave_speed = 0.0001f;
float moveFactor = 0;

// lighting
glm::vec3 lightPos(0, 3, 0);
glm::vec3 light_Color(1, 1, 1);
glm::vec3 poolMove(0.0, -1.0, 0.0);

int main()
{

	// --------------- set glfw, glad ------------------

	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "CG Final Project", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback); // set mouse callback
	glfwSetScrollCallback(window, scroll_callback); // set scroll callback
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);	//隐藏鼠标
													// glad: load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// ------- configure global opengl state -------
	glEnable(GL_CULL_FACE);

	glEnable(GL_CLIP_DISTANCE0);

	// ------------------ shaders ------------------
	Shader waterShader("./water.vs", "./water.frag");
	Shader wallShader("./wallShader.vs", "./wallShader.frag");
	Shader screenShader("./screenShader.vs", "./screenShader.frag");

	// ----------------- data processing ----------------

	// vertex data
	float water[] = { // all y set to 0
		
		-1, 1,
		1, 1,
		1, -1,
		-1, 1,
		1, -1,
		-1, -1
	};

	float wall[] = {
		// position texture
		-2, 2, -5,   0, 1,
		-2, -2, -5,  0, 0,
		2, -2, -5,   1, 0,
		2, -2, -5,   1, 0,
		2, 2, -5,    1, 1,
		-2, 2, -5,   0, 1,

		2, 2, 5,     0, 1,
		2, -2, 5,    0, 0,
		-2, -2, 5,   1, 0,
		-2, -2, 5,   1, 0,
		-2, 2, 5,    1, 1,
		2, 2, 5,     0, 1,

		-2, 2, 5,    0, 1,
		-2, -2, 5,   0, 0,
		-2, -2, -5,  1, 0,
		-2, -2, -5,  1, 0,
		-2, 2, -5,   1, 1,
		-2, 2, 5,    0, 1,

		2, 2, -5,    0, 1,
		2, -2, -5,   0, 0,
		2, -2, 5,    1, 0,
		2, -2, 5,    1, 0,
		2, 2, 5,     1, 1,
		2, 2, -5,    0, 1
	};

	float floor[] = {
		-2, -2, -5,  0, 1,
		-2, -2, 5,   0, 0,
		2, -2, 5,    1, 0,
		2, -2, 5,    1, 0,
		2, -2, -5,   1, 1,
		-2, -2, -5,  0, 1,
	};

	float quadVertices[] = { // for rendering to screen space
		// positions   // texCoords
		/*
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		1.0f, -1.0f,  1.0f, 0.0f,
		1.0f,  1.0f,  1.0f, 1.0f
		*/
		-1.0f,  1.0f,  0.0f, 0.0f,
		-1.0f, -1.0f,  0.0f, 1.0f,
		1.0f, -1.0f,  1.0f, 1.0f,

		-1.0f,  1.0f,  0.0f, 0.0f,
		1.0f, -1.0f,  1.0f, 1.0f,
		1.0f,  1.0f,  1.0f, 0.0f

	};

	// water & wall
	unsigned int waterVBO, waterVAO, wallVBO, floorVBO; // NOTE: wallVAO is declared as global var for convenience
	glGenVertexArrays(1, &waterVAO);
	glGenBuffers(1, &waterVBO);
	glGenVertexArrays(1, &wallVAO);
	glGenBuffers(1, &wallVBO);
	glGenVertexArrays(1, &floorVAO);
	glGenBuffers(1, &floorVBO);

	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(waterVAO);
	glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(water), water, GL_STATIC_DRAW);
	// position attribute
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(wallVAO);
	glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall), wall, GL_STATIC_DRAW);
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(floorVAO);
	glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floor), floor, GL_STATIC_DRAW);
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	


	// -------------- textures ---------------------
	texture1 = loadTexture("textures/marble.bmp");
	texture2 = loadTexture("textures/wood.bmp");
	// load DuDv texture 
	DuDvTexture = loadTexture("textures/dudvMap.png");
	// load normal texture 
	normalTexture = loadTexture("textures/normalMap.png");

	// ------------ shader configuration ---------------
	wallShader.use();
	wallShader.setInt("texture1", 0);

	screenShader.use();
	screenShader.setInt("screenTexture", 0);

	
	// ----------- frame buffer configuration ----------

	unsigned int reflectionFBO = initializeReflectionFBO();
	unsigned int refractionFBO = initializeRefractionFBO();


	// --------------- drawing mode ---------------------

	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // uncomment to see that we only rendered a single quad in screen space using FBOs

	// --------------- render loop ------------------------
	float framX = 1600;
	while (!glfwWindowShouldClose(window))
	{
		glEnable(GL_CLIP_DISTANCE0); // enable clip distance
		// timing
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		framX++;
		// input
		processInput(window);

		// ------------------ 1st pass ---------------

		
		// render reflection texture
		glBindFramebuffer(GL_FRAMEBUFFER, reflectionFBO);
		float distance = 2 * (camera.Position.y - 0);
		//camera.Position.y -= distance;
		camera.Pitch = -camera.Pitch; // invert camera pitch
		//camera.invertPitch();
		renderScene(wallShader, reflect_plane);
		// reset camera back to original position
		//camera.Position.y += distance;
		camera.Pitch = -camera.Pitch;
		//camera.invertPitch();


		// render refraction texture
		glBindFramebuffer(GL_FRAMEBUFFER, refractionFBO);
		//renderScene(wallShader, wallVAO, refract_plane);
		renderScene(wallShader, refract_plane);

		



		// render to screen
		glDisable(GL_CLIP_DISTANCE0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0); // now bind back to default framebuffer								  //renderScene(wallShader, wallVAO, plane);
		renderScene(wallShader, plane);

		//std::cout << camera.Position.x << "  " << camera.Position.y << "  " << camera.Position.z << "\n";
		// render water
		waterShader.use();
		waterShader.setInt("reflectionTexture", 0);
		waterShader.setInt("refractionTexture", 1);
		waterShader.setInt("dudvMap", 2);
		waterShader.setInt("normalMap", 3);

		// pass camera position 
		waterShader.setVec3("cameraPosition", camera.Position);
		waterShader.setVec3("lightPosition", lightPos);
		waterShader.setVec3("lightColor", light_Color);

		// wave 
		moveFactor += wave_speed * framX * deltaTime;		
		if (moveFactor >= 1) {
			moveFactor -= 1;
			framX = 1600;
		}
		waterShader.setFloat("moveFactor", moveFactor);


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, reflectionColorBuffer);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, refractionColorBuffer);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, DuDvTexture);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, normalTexture);

		glBindVertexArray(waterVAO);

		// do transformations
		
		glm::mat4 projection;
		projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		waterShader.setMat4("projection", projection);
		// camera/view transformation
		glm::mat4 view = camera.GetViewMatrix();
		waterShader.setMat4("view", view);
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, poolMove);
		model = glm::scale(model, glm::vec3(2.0, 1.0, 5.0));

		waterShader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// deallocate resources
	glDeleteVertexArrays(1, &waterVAO);
	glDeleteBuffers(1, &waterVBO);
	//glDeleteFramebuffers(1, &reflectionFBO);
	//glDeleteFramebuffers(1, &refractionFBO);
	// ToDo: Delete textures, rbo

	// glfw: terminate, clearing all previously allocated GLFW resources.
	glfwTerminate();
	return 0;
}

// reflectionColorBuffer set as global var for convenience
unsigned int initializeReflectionFBO()
{
	// reflection frame buffer
	unsigned int fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	// create a color attachment texture
	unsigned int depthRenderBuffer;
	glGenTextures(1, &reflectionColorBuffer);
	glBindTexture(GL_TEXTURE_2D, reflectionColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, reflectionColorBuffer, 0);
	// create a depth render buffer attachment
	glGenRenderbuffers(1, &depthRenderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer); // now actually attach it
																										// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Reflection framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return fbo;
}

unsigned int initializeRefractionFBO()
{
	// refraction frame buffer
	unsigned int fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	// create a color attachment texture
	unsigned int depthBuffer;
	glGenTextures(1, &refractionColorBuffer);
	glBindTexture(GL_TEXTURE_2D, refractionColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, refractionColorBuffer, 0);
	// create a depth attachment texture
	glGenTextures(1, &depthBuffer);
	glBindTexture(GL_TEXTURE_2D, depthBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBuffer, 0);
	// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Refraction framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return fbo;
}

// draw everything aside from water
void renderScene(Shader wallShader, float clipPlane[4])
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture1); // marble texture

	wallShader.use();
	// pass clip plane
	GLuint plane_location = glGetUniformLocation(wallShader.ID, "plane");
	glUniform4fv(plane_location, 1, clipPlane);
	// do transformations
	glm::mat4 projection;
	projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	wallShader.setMat4("projection", projection);
	// camera/view transformation
	glm::mat4 view = camera.GetViewMatrix();
	wallShader.setMat4("view", view);

	glBindVertexArray(wallVAO);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, poolMove);
	wallShader.setMat4("model", model);
	glDrawArrays(GL_TRIANGLES, 0, 24);

	// draw floor
	glBindVertexArray(floorVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture2); // floor texture
	glDrawArrays(GL_TRIANGLES, 0, 6);

}



// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window)
{
	float keyViewMoveSpeed = 2;
	//Camera old_camera = camera;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.ProcessKeyboard(FORWARD, deltaTime);
		//our_maze->doCollisions(camera, objectSize, FORWARD, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.ProcessKeyboard(BACKWARD, deltaTime);
		//our_maze->doCollisions(camera, objectSize, BACKWARD, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.ProcessKeyboard(LEFT, deltaTime);
		//our_maze->doCollisions(camera, objectSize, LEFT, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.ProcessKeyboard(RIGHT, deltaTime);
		//our_maze->doCollisions(camera, objectSize, RIGHT, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera.ProcessMouseMovement(-keyViewMoveSpeed, 0);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camera.ProcessMouseMovement(keyViewMoveSpeed, 0);

	// jump
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		camera.ProcessKeyboard(JUMP, deltaTime);
	}

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

//普通2D纹理
unsigned int loadTexture(char const * path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);	//过滤方式
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}
	return textureID;
}


