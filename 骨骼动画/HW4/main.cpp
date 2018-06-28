#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "IL\il.h"
#include "IL\ilu.h"
#include "IL\ilut.h"
#include <iostream>
#include "learnOpenGL\Camera.h"
#include "learnOpenGL\shader.h"
#include "learnOpenGL\stb_image.h"
#include "learnOpenGL\Maze.h"
// #include "learnOpenGL\model.h"
#include "assimp\Importer.hpp"
#include "assimp\scene.h"
#include "assimp\postprocess.h"
#include "Triangle.h"
#include <vector>
using namespace std;

//窗口大小
int SCR_WIDTH = 1280;
int SCR_HEIGHT = 720;

//定义摄像机
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));


int main()
{
	//初始化
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	//创建窗口
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Maze", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	//将此窗口的上下文设置为当前线程的主上下文
	glfwMakeContextCurrent(window);
	
	//GLAD加载
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	ilInit(); // init image load library
	iluInit();
	ilutInit();
	ilutRenderer(ILUT_OPENGL);
	ilEnable(IL_ORIGIN_SET);
	// OpenGL have the 0.0 coordinate on the bottom y-axis, but images usually have 0.0 at the top of the y-axis.
	ilOriginFunc(IL_ORIGIN_UPPER_LEFT); // 0, 0 textures in upper left side

										// MIX_DEFAULT_FREQUENCY = sample rate = frequensy = speed playing
	if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 4069) == -1)
		cout << "Mixer NOT init !!" << endl;
	Mix_VolumeMusic(1);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	//骨骼动画
	Triangle triangle;
	triangle.init();

	//渲染循环
	while (!glfwWindowShouldClose(window))
	{

		// render
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 骨骼动画
		triangle.update(camera);
		triangle.render(camera);


		//渲染
		//ImGui::Render();
		//ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}