//#define GLEW_STATIC
//#include <GL/glew.h>
#include "glad/glad.h"
#include <GL/glfw3.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <iostream>
#include <ctime>
#include "learnOpenGL/Camera.h"
#include "learnOpenGL/shader.h"
#include "learnOpenGL/stb_image.h"
using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void glfw_error_callback(int error, const char* description);
void processInput(GLFWwindow *window, float deltaTime);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void RenderScene(Shader &shader, unsigned int & planeVAO, unsigned int & cubeVAO);
GLuint create_texture(GLchar* path);

//窗口大小
int window_width = 800;
int window_height = 800;
//帧与帧时间差
float deltaTime = 0.0f;
float lastFrame = 0.0f;
//初始鼠标位置
float lastx = window_width / 2;
float lasty = window_height / 2;
bool firstMouse = true;
//摄像机类，初始摄像机设置
Camera camera(glm::vec3(0.0f, 2.5f, 4.0f));
//立方体的8个顶点
float vertices[] = {
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
	0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
	0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,

	0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
	0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f
};
//地板平面
float planeVertices[]{
	25.0f, -0.5f,  25.0f, 0.0f, 1.0f, 0.0f, 25.0f,  0.0f,
	-25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f,  0.0f, 25.0f,
	-25.0f, -0.5f,  25.0f, 0.0f, 1.0f, 0.0f,  0.0f,  0.0f,

	25.0f, -0.5f,  25.0f, 0.0f, 1.0f, 0.0f, 25.0f,  0.0f,
	25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 25.0f,
	-25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f,  0.0f, 25.0f
};

//灯源位置
glm::vec3 lightPos(-2.0f, 3.0f, -1.0f);

int main() {
	glfwSetErrorCallback(glfw_error_callback);
	//初始化GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);	//配置GLFW,版本号3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);	//核心渲染模式
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	//创建窗口
	GLFWwindow* window = glfwCreateWindow(window_width, window_height, "Shadowing Mapping", NULL, NULL);	//窗口宽、高，命名
	if (window == NULL) {
		cout << "Fail to create GLFW window" << endl;
		glfwTerminate();
		return -1;
	}
	//将此窗口的上下文设置为当前线程的主上下文
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);	//隐藏光标
	//glfwSetCursorPosCallback(window, mouse_callback);				//鼠标移动监控
	//GLAD加载
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		cout << "Fail to initialize GLAD" << endl;
		return -1;
	}
	//初始化GLEW
	//glewExperimental = GL_TRUE;
	//glewInit();
	glViewport(0, 0, window_width, window_height);
	//深度测试
	glEnable(GL_DEPTH_TEST);

	//绑定imGui
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui_ImplGlfwGL3_Init(window, true);
	ImGui::StyleColorsDark();

	//利用shader_m类创建着色器
	Shader lightShader("shader/light.vs", "shader/light.fs");
	Shader cubeShader("shader/cube_shader.vs", "shader/cube_shader.fs");
	Shader depthShader("shader/depth_shader.vs", "shader/depth_shader.fs");
	cubeShader.use();
	cubeShader.setInt("diffuseTexture", 0);
	cubeShader.setInt("shadowMap", 1);

	//生成地板平面VAO，VBO
	unsigned int planeVAO, planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);	//将缓冲绑定
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);	//顶点位置
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));	//法向量
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));	//纹理
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);

	//生成立方体VAO,VBO对象
	unsigned int cubeVAO, VBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);	//将缓冲绑定
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);	//将vertices赋值到缓冲中
	glBindVertexArray(cubeVAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);	//顶点位置
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));	//法向量
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));	//纹理
	glEnableVertexAttribArray(2);

	//生成顶点光源VAO,VBO对象
	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);	//将缓冲绑定
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);	//顶点位置
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	//立方体纹理
	GLuint cubeTexture = create_texture("./img/wall.jpg");

	//深度贴图的帧缓冲
	GLuint depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	//创建一个2D纹理
	const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	GLuint depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);	//环绕方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	//过滤方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);	//不进行颜色数据的渲染,我们只需要深度信息
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	bool show_menu = true;	//创建菜单
	float fov = 45.0, near_plat1 = 0.1, far_plat1 = 100.0;	//投影的系数
	float ambientStrength = 0.15;	//环境光强度
	int Shininess = 64;				//反光度
	float specularStrength = 1.0;	//镜面反射的强度
	float diffStrength = 1.0;		//漫反射强度
	static int choice = 0;			//选择光源的投影模式
	static int lightPosChange = 0;	//光源不动or不停动
									//渲染循环
	while (!glfwWindowShouldClose(window))
	{
		//计算上一帧与这一帧的时间差
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		//键盘输入
		processInput(window, deltaTime);
		glClearColor(0.1, 0.1, 0.1, 1.0);

		//选择是否改变光源位置
		if (lightPosChange == 1) {
			lightPos.x = 1.0f + sin(glfwGetTime()) * 2.0f;
			lightPos.y = sin(glfwGetTime() / 2.0f) * 1.0f + 1.2f;
			lightPos.z = sin(glfwGetTime()) * 2.0f - 1.0f;
		}

		//1. 以光源视角渲染场景，得到深度纹理图（正交投影or透视投影）
		glm::mat4 lightProjection;
		GLfloat near_plane = 1.0f, far_plane = 25.0f;
		if (choice == 0) {	//正交投影
			lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		}
		else {				//透视投影
			lightProjection = glm::perspective(glm::radians(45.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
		}
		glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));	//从光源看向场景中央
		glm::mat4 lightSpaceMatrix = lightProjection * lightView;	//变换矩阵T
		depthShader.use();
		depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);	//渲染深度贴图
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		RenderScene(depthShader, planeVAO, cubeVAO);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//2. 正常渲染（透视投影）
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);	//重置视角
		glm::mat4 model, view, projection;
		//旋转一下以便观察
		//model = glm::rotate(model, glm::radians(30.0f), glm::vec3(1.0f, 1.0f, 0.0f));
		view = camera.getViewMatrix();	//此时的摄像机观察矩阵,运用Camera类
		projection = glm::perspective(glm::radians(fov), (float)display_w / (float)display_h, near_plat1, far_plat1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		cubeShader.use();
		cubeShader.setFloat("ambientStrength", ambientStrength);
		cubeShader.setFloat("specularStrength", specularStrength);
		cubeShader.setFloat("diffStrength", diffStrength);
		cubeShader.setInt("Shininess", Shininess);
		cubeShader.setVec3("lightPos", lightPos);
		cubeShader.setVec3("viewPos", camera.cameraPos);
		//cubeShader.setMat4("model", model);
		cubeShader.setMat4("view", view);
		cubeShader.setMat4("projection", projection);
		cubeShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		//ortho or perspective
		cubeShader.setInt("ortho_or_pespective", choice);
		cubeShader.setFloat("near_plane", near_plane);
		cubeShader.setFloat("far_plane", far_plane);
		//绑定纹理
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cubeTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		RenderScene(cubeShader, planeVAO, cubeVAO);

		//绘制灯源
		lightShader.use();
		//将灯源位移一下并缩小
		model = glm::mat4();
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.1f));
		lightShader.setMat4("model", model);
		lightShader.setMat4("view", view);
		lightShader.setMat4("projection", projection);
		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glfwPollEvents();
		//GUI界面
		ImGui_ImplGlfwGL3_NewFrame();
		ImGui::Begin("Menu", &show_menu);
		if (ImGui::CollapsingHeader("Choose the light position fixed or changing")) {
			ImGui::RadioButton("fixed", &lightPosChange, 0);
			ImGui::RadioButton("changing", &lightPosChange, 1);
		}
		if (ImGui::CollapsingHeader("Choose projection of the light")) {
			ImGui::RadioButton("ortho", &choice, 0);
			ImGui::RadioButton("perspective", &choice, 1);
		}
		if (ImGui::CollapsingHeader("Phong Shading")) {
			ImGui::SliderFloat("ambient strenght", &ambientStrength, 0.0, 1.0, "%.2f");		//环境光强度
			ImGui::SliderFloat("diff strenght", &diffStrength, 0.0, 1.0, "%.2f");		//漫反射强度
			ImGui::SliderFloat("specular strength", &specularStrength, 0.0, 1.0, "%.2f");	//镜面反射强度
			ImGui::SliderInt("Shininess", &Shininess, 0, 500);			//反光度
		}
		ImGui::End();
		//渲染
		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	//终止
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &lightVAO);
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteVertexArrays(1, &planeVBO);
	glDeleteBuffers(1, &VBO);
	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	return 0;
}


//辅助函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}
static void glfw_error_callback(int error, const char* description) {
	fprintf(stderr, "Error %d: %s\n", error, description);
}
//键盘输入
void processInput(GLFWwindow *window, float deltaTime)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {		//W键
		camera.moveForward(deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {		//S键
		camera.moveBack(deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {		//A键
		camera.moveLeft(deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {		//D键
		camera.moveRight(deltaTime);
	}
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	//第一次鼠标移动
	if (firstMouse) {
		lastx = xpos;
		lasty = ypos;
		firstMouse = false;
	}
	//鼠标位移
	float xoffset = -(xpos - lastx);
	float yoffset = -(lasty - ypos);
	lastx = xpos;
	lasty = ypos;
	camera.rotate(xoffset, yoffset);
}
//渲染(地板+立方体)
void RenderScene(Shader &shader, unsigned int & planeVAO, unsigned int & cubeVAO) {
	//地板平面（用于显示阴影）
	glm::mat4 model;
	shader.setMat4("model", model);
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	//立方体（2个）
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0));
	shader.setMat4("model", model);
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	model = glm::mat4();
	model = glm::translate(model, glm::vec3(2.0f, 0.5f, -0.5));
	model = glm::rotate(model, 60.0f, glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.5));
	shader.setMat4("model", model);
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

// 加载&创建纹理
GLuint create_texture(GLchar* path) {
	GLuint Texture;
	glGenTextures(1, &Texture);
	glBindTexture(GL_TEXTURE_2D, Texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	//环绕方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);	//过滤方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	glBindTexture(GL_TEXTURE_2D, 0);	//解绑
	stbi_image_free(data);
	return Texture;
}
