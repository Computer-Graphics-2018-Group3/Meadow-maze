#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "stb/stb_image.h"

#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>
#include "Triangle.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>
#include <string>

#include "maze/camera.h"
#include "maze/shader.h"
#include "maze/model.h"
#include "maze/maze.h"
#include "maze/cloth.h"
#include "maze/wave.h"
#include "maze/particle_generator.h"

using namespace std;

//freetype
/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
	GLuint TextureID;   // ID handle of the glyph texture
	glm::ivec2 Size;    // Size of glyph
	glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
	GLuint Advance;    // Horizontal offset to advance to next glyph
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void cursor_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window, Maze* ourMaze);
unsigned int loadTexture(const char* path);
unsigned int loadCubemap(vector<std::string> faces);
void RenderScene(Shader& shader, unsigned int& planeVAO, unsigned int& cubeVAO, Maze* ourMaze);
void RenderText(Shader &shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);
void RenderWall(Shader& shader, unsigned int& cubeVAO);

// settings
int SCR_WIDTH = 1280;
int SCR_HEIGHT = 720;

// camera
Camera camera(glm::vec3(0.0f, 3.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;

float objectSize = 0.04f;
bool firstMouse = true;

// light source
glm::vec3 lightPos(-21.0f, 35.0f, -35.0f);

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// map
bool mazeMap = true;

// font
std::map<GLchar, Character> Characters;
GLuint tVAO, tVBO;

// set up vertex data (and buffer(s)) and configure vertex attributes
// ------------------------------------------------------------------
float cubeVertices[] = {
	// positions           // normals			 // texCoords   // tangent 		      // bitangent
	-0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f,  0.0f,   1.0f,  0.0f,  0.0f,   0.0f,  1.0f,  0.0f,
	 0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f,  0.0f,   1.0f,  0.0f,  0.0f,   0.0f,  1.0f,  0.0f,
	 0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f,  1.0f,   1.0f,  0.0f,  0.0f,   0.0f,  1.0f,  0.0f,
	 0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f,  1.0f,   1.0f,  0.0f,  0.0f,   0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f,  1.0f,   1.0f,  0.0f,  0.0f,   0.0f,  1.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f,  0.0f,   1.0f,  0.0f,  0.0f,   0.0f,  1.0f,  0.0f,

	-0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f,  0.0f,   1.0f,  0.0f,  0.0f,   0.0f,  1.0f,  0.0f,
	 0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f,  0.0f,   1.0f,  0.0f,  0.0f,   0.0f,  1.0f,  0.0f,
	 0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f,  1.0f,   1.0f,  0.0f,  0.0f,   0.0f,  1.0f,  0.0f,
	 0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f,  1.0f,   1.0f,  0.0f,  0.0f,   0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f,  1.0f,   1.0f,  0.0f,  0.0f,   0.0f,  1.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f,  0.0f,   1.0f,  0.0f,  0.0f,   0.0f,  1.0f,  0.0f,

	-0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   0.0f,  1.0f,   0.0f,  0.0f, -1.0f,   0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   1.0f,  1.0f,   0.0f,  0.0f, -1.0f,   0.0f,  1.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   1.0f,  0.0f,   0.0f,  0.0f, -1.0f,   0.0f,  1.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   1.0f,  0.0f,   0.0f,  0.0f, -1.0f,   0.0f,  1.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   0.0f,  0.0f,   0.0f,  0.0f, -1.0f,   0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   0.0f,  1.0f,   0.0f,  0.0f, -1.0f,   0.0f,  1.0f,  0.0f,

	 0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   0.0f,  1.0f,   0.0f,  0.0f, -1.0f,   0.0f,  1.0f,  0.0f,
	 0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f,  1.0f,   0.0f,  0.0f, -1.0f,   0.0f,  1.0f,  0.0f,
	 0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f,  0.0f,   0.0f,  0.0f, -1.0f,   0.0f,  1.0f,  0.0f,
	 0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f,  0.0f,   0.0f,  0.0f, -1.0f,   0.0f,  1.0f,  0.0f,
	 0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   0.0f,  0.0f,   0.0f,  0.0f, -1.0f,   0.0f,  1.0f,  0.0f,
	 0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   0.0f,  1.0f,   0.0f,  0.0f, -1.0f,   0.0f,  1.0f,  0.0f,

	-0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f,  1.0f,   1.0f,  0.0f,  0.0f,   0.0f,  0.0f, -1.0f,
	 0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   1.0f,  1.0f,   1.0f,  0.0f,  0.0f,   0.0f,  0.0f, -1.0f,
	 0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   1.0f,  0.0f,   1.0f,  0.0f,  0.0f,   0.0f,  0.0f, -1.0f,
	 0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   1.0f,  0.0f,   1.0f,  0.0f,  0.0f,   0.0f,  0.0f, -1.0f,
	-0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   0.0f,  0.0f,   1.0f,  0.0f,  0.0f,   0.0f,  0.0f, -1.0f,
	-0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f,  1.0f,   1.0f,  0.0f,  0.0f,   0.0f,  0.0f, -1.0f,

	-0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f,  1.0f,   1.0f,  0.0f,  0.0f,   0.0f,  0.0f, -1.0f,
	 0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   1.0f,  1.0f,   1.0f,  0.0f,  0.0f,   0.0f,  0.0f, -1.0f,
	 0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   1.0f,  0.0f,   1.0f,  0.0f,  0.0f,   0.0f,  0.0f, -1.0f,
	 0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   1.0f,  0.0f,   1.0f,  0.0f,  0.0f,   0.0f,  0.0f, -1.0f,
	-0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   0.0f,  0.0f,   1.0f,  0.0f,  0.0f,   0.0f,  0.0f, -1.0f,
	-0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f,  1.0f,   1.0f,  0.0f,  0.0f,   0.0f,  0.0f, -1.0f,
};

float skyboxVertices[] = {
	// positions          
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

float planeVertices[] = {
	 50.0f, -0.5f,  50.0f, 0.0f, 1.0f, 0.0f, 50.0f,  0.0f,
	-50.0f, -0.5f, -50.0f, 0.0f, 1.0f, 0.0f,  0.0f, 50.0f,
	-50.0f, -0.5f,  50.0f, 0.0f, 1.0f, 0.0f,  0.0f,  0.0f,

	 50.0f, -0.5f,  50.0f, 0.0f, 1.0f, 0.0f, 50.0f,  0.0f,
	 50.0f, -0.5f, -50.0f, 0.0f, 1.0f, 0.0f, 50.0f, 50.0f,
	-50.0f, -0.5f, -50.0f, 0.0f, 1.0f, 0.0f,  0.0f, 50.0f
};

float quadVertices[] = {   // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
};

glm::vec3 treePos[] = {
	//前后
	glm::vec3(-2.0f, -0.5f, -20.0f),
	glm::vec3(-3.0f, -0.5f, -22.0f),
	glm::vec3(3.0f, -0.5f, -19.0f),
	glm::vec3(14.0f, -0.5f, 2.0f),
	glm::vec3(18.0f, -0.5f, -20.0f),
	glm::vec3(10.0f, -0.5f, -18.0f),
	glm::vec3(6.0f, -0.5f, 4.0f),
	glm::vec3(1.0f, -0.5f, -18.0f),
	glm::vec3(4.0f, -0.5f, -24.0f),
	glm::vec3(7.0f, -0.5f, -26.0f),
	glm::vec3(9.0f, -0.5f, -25.0f), // 11

	//左右
	glm::vec3(-3.0f, -0.5f, -4.0f),
	glm::vec3(-4.0f, -0.5f, -8.0f),
	glm::vec3(-5.0f, -0.5f, -20.0f),
	glm::vec3(-2.0f, -0.5f, -24.0f),
	glm::vec3(-3.0f, -0.5f, 2.0f),
	glm::vec3(-4.0f, -0.5f, -18.0f),
	glm::vec3(-5.0f, -0.5f, -14.0f),
	glm::vec3(-2.0f, -0.5f, -10.0f), // 19

	glm::vec3(20.0f, -0.5f, -4.0f),
	glm::vec3(18.0f, -0.5f, -8.0f),
	glm::vec3(24.0f, -0.5f, -20.0f),
	glm::vec3(19.0f, -0.5f, -24.0f),
	glm::vec3(17.0f, -0.5f, 2.0f),
	glm::vec3(22.0f, -0.5f, -18.0f),
	glm::vec3(20.0f, -0.5f, -14.0f),
	glm::vec3(25.0f, -0.5f, -10.0f) // 27
};

float stencilVertices[] = {
    // positions          
     5.0f, -0.5f,  5.0f,
    -5.0f, -0.5f,  5.0f,
    -5.0f, -0.5f, -5.0f,

     5.0f, -0.5f,  5.0f,
    -5.0f, -0.5f, -5.0f,
     5.0f, -0.5f, -5.0f
};

// Cloth
Cloth cloth;

// Wave
#define MESH_RESOLUTION 49
// Mesh resolution
int N = MESH_RESOLUTION;
int M = MESH_RESOLUTION;
float L_x = 70;
float L_z = 45;
// 振幅
float A = 3e-7f * 2.25;
// Wind speed	风速度
float V = 30;
// Wind direction	方向
glm::vec2 omega(1, 1);

Wave wave_model(N, M, L_x, L_z, omega, V, A, 1);

bool M_KEY_ACT = true;


int main()
{
    // glfw: initialize and configure
    // ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Maze", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, cursor_callback);
	glfwSetScrollCallback(window, scroll_callback);
	// glfwSetMouseButtonCallback(window, mouse_callback);

    // tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
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

	// skeleton animation
	Triangle triangle;
	triangle.init();

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);

    // build and compile shaders
    // -------------------------
	Shader cubeShader("shader/cube_shader.vs", "shader/cube_shader.fs");
	Shader depthShader("shader/depth_shader.vs", "shader/depth_shader.fs");
	Shader skyboxShader("shader/skybox_shader.vs", "shader/skybox_shader.fs");
	Shader screenShader("shader/screen_shader.vs", "shader/screen_shader.fs");
	Shader clothShader("shader/cloth_shader.vs", "shader/cloth_shader.fs");
	Shader modelShader("shader/model_shader.vs", "shader/model_shader.fs");
	Shader particleShader("shader/particle_shader.vs", "shader/particle_shader.fs");
	Shader textShader("shader/text_shader.vs", "shader/text_shader.fs");
	Shader stencilShader("shader/stencil_shader.vs", "shader/stencil_shader.fs");

	// shader configuration
	// --------------------
	cubeShader.use();
	cubeShader.setInt("diffuseTexture", 0);
	cubeShader.setInt("shadowMap", 1);
	cubeShader.setInt("normalMap", 2);

	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

    screenShader.use();
    screenShader.setInt("screenTexture", 0);

    particleShader.use();
    particleShader.setInt("sprite", 0);

	// cube VAO
	unsigned int cubeVAO, cubeVBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
	glEnableVertexAttribArray(4);
	glBindVertexArray(0);

	// skybox VAO
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	// plane VAO
	unsigned int planeVAO, planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);

    // setup screen VAO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // setup stencil VAO
    unsigned int stencilVAO, stencilVBO;
    glGenVertexArrays(1, &stencilVAO);
    glGenBuffers(1, &stencilVBO);
    glBindVertexArray(stencilVAO);
    glBindBuffer(GL_ARRAY_BUFFER, stencilVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(stencilVertices), &stencilVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    // Load textures
	unsigned int planeTexture = loadTexture("skybox/ame_siege/siege_dn.tga");
	unsigned int cubeTexture = loadTexture("img/wall_test.jpg");
	unsigned int normalMap = loadTexture("img/wall_normal.jpg");
	unsigned int particleTexture = loadTexture("img/snow.png");
	vector<std::string> faces
	{
		"skybox/ame_siege/siege_ft.tga",
		"skybox/ame_siege/siege_bk.tga",
		"skybox/ame_siege/siege_up.tga",
		"skybox/ame_siege/siege_dn.tga",
		"skybox/ame_siege/siege_rt.tga",
		"skybox/ame_siege/siege_lf.tga"
	};
	unsigned int cubemapTexture = loadCubemap(faces);

    // configure MSAA framebuffer
    // --------------------------
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // create a multisampled color attachment texture
    unsigned int textureColorBufferMultiSampled;
    glGenTextures(1, &textureColorBufferMultiSampled);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, SCR_WIDTH, SCR_HEIGHT, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled, 0);
    // create a (also multisampled) renderbuffer object for depth and stencil attachments
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // configure second post-processing framebuffer
    unsigned int intermediateFBO;
    glGenFramebuffers(1, &intermediateFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
    // create a color attachment texture
    unsigned int screenTexture;
    glGenTextures(1, &screenTexture);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);  // we only need a color buffer

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Intermediate framebuffer is not complete!" << endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Configure depth map FBO
	const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	GLuint depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
    // Create depth texture
	GLuint depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // maze: initialize and configure
    // ------------------------------
	Maze* ourMaze = new Maze();
	ourMaze->Init(8, 8);
	ourMaze->autoGenerateMaze();
	glm::vec3 exitPos = ourMaze->getExit();

	// load models
    // -----------
    Modelt post("./model/post/rv_lamp_post_2.obj");
    Modelt palm("./model/Palm_01/Palm_01.obj");
    Modelt tree("./model/tree/Tree.obj");
    Modelt statue("./model/Umbreon/UmbreonLowPoly.obj");

	// particle configuration
	ParticleGenerator Particles(particleShader, particleTexture, 500);

	// Wave
	// Speed of wind, direction of wind
	float modelScale = 0.03f;
	wave_model.setShader("shader/surface_shader.vs", "shader/surface_shader.fs");
	wave_model.initBufferObjects();
	glm::vec3 movePos(-2.0f, -0.3f, 3.0f);			// 控制wave位置

	// freetype
	// Compile and setup the shader
	glm::mat4 projectiont = glm::ortho(0.0f, static_cast<GLfloat>(SCR_WIDTH), 0.0f, static_cast<GLfloat>(SCR_HEIGHT));
	textShader.use();
	textShader.setMat4("projection", projectiont);
	textShader.setInt("text", 0);
	// FreeType
	FT_Library ft;
	// All functions return a value different than 0 whenever an error occurred
	if (FT_Init_FreeType(&ft))
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
	// Load font as face
	FT_Face face;
	if (FT_New_Face(ft, "/Users/Eddie/Desktop/Maze_v9/maze_text.ttf", 0, &face))
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
	// Set size to load glyphs as
	FT_Set_Pixel_Sizes(face, 0, 48);
	// Disable byte-alignment restriction
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	for (GLubyte c = 0; c < 128; c++)
	{
		// Load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}
		// Generate texture
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// Set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Now store character for later use
		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			static_cast<GLuint>(face->glyph->advance.x)
		};
		Characters.insert(std::pair<GLchar, Character>(c, character));
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	// Destroy FreeType once we're finished
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
	// Configure VAO/VBO for texture quads
	glGenVertexArrays(1, &tVAO);
	glGenBuffers(1, &tVBO);
	glBindVertexArray(tVAO);
	glBindBuffer(GL_ARRAY_BUFFER, tVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


    // Render loop
	while (!glfwWindowShouldClose(window))
	{
        // Set frame time
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

        // Check and call events
		processInput(window, ourMaze);

		// Render
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		
        // Render depth of scene to texture (from light's perspective)
        // get light projection/view matrix.
		glm::mat4 lightProjection;
		GLfloat near_plane = 40.0f, far_plane = 80.0f;
		lightProjection = glm::perspective(glm::radians(50.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
		// lightProjection = glm::ortho(-25.0f, 25.0f, -25.0f, 25.0f, near_plane, far_plane);
		glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
		glm::mat4 lightSpaceMatrix = lightProjection * lightView;

		glm::mat4 model;
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        // draw the original scene
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0x00);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		// - now render scene from light's point of view
		depthShader.use();
		depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);
			RenderScene(depthShader, planeVAO, cubeVAO, ourMaze);
			RenderWall(depthShader, cubeVAO);
			model = glm::translate(glm::mat4(), glm::vec3(0.1f, 0.4f, 0.0f)+exitPos);
			model = glm::scale(model, glm::vec3(0.02f, 0.016f, 0.016f));
       		depthShader.setMat4("model", model);
       		cloth.update(deltaTime);
       		cloth.draw();
	        model = glm::translate(glm::mat4(), glm::vec3(0.0f, -0.5f, 0.0f)+exitPos);
	        model = glm::scale(model, glm::vec3(0.06f, 0.06f, 0.06f));
       		depthShader.setMat4("model", model);
       		post.Draw(depthShader);
			model = glm::translate(glm::mat4(), glm::vec3(0.0f, -0.25f, 1.0f));
			model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
			depthShader.setMat4("model", model);
			statue.Draw(depthShader);
			model = glm::translate(model, glm::vec3(10.0f, 0.0f, 0.0f));
			depthShader.setMat4("model", model);
			statue.Draw(depthShader);
			for (int i = 0; i <27; i++) {
				model = glm::translate(glm::mat4(), treePos[i]);
				model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
				depthShader.setMat4("model", model);
				tree.Draw(depthShader);
			}
			model = glm::translate(glm::mat4(), glm::vec3(-2.0f, -0.3f, 0.2f));
			model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
			depthShader.setMat4("model", model);
			palm.Draw(depthShader);
			triangle.update(camera);
			triangle.render(camera, &depthShader); 
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Render scene as normal 
		glfwGetFramebufferSize(window, &SCR_WIDTH, &SCR_HEIGHT);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        // draw scene as normal in multisampled buffers
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glStencilFunc(GL_ALWAYS, 10, 0xFF);
        glStencilMask(0xFF);
    	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

		// update the stencil buffer
		stencilShader.use();
        stencilShader.setMat4("view", view);
        stencilShader.setMat4("projection", projection);
		glBindVertexArray(stencilVAO);
		model = glm::translate(glm::mat4(), glm::vec3(-2.0f, 0.2f, 3.0f));
		model = glm::scale(model, glm::vec3(0.185f, 1.0f, 0.12f));
		stencilShader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

        glStencilFunc(GL_ALWAYS, 0, 0xFF);
        glStencilMask(0xFF);

		cubeShader.use();
		cubeShader.setMat4("view", view);
        cubeShader.setMat4("projection", projection);
		RenderWall(cubeShader, cubeVAO);

		modelShader.use();
        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);
        model = glm::translate(glm::mat4(), glm::vec3(-3.0f, -0.5f, 2.0f));
        modelShader.setMat4("model", model);
        tree.Draw(modelShader);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0x00);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

        // draw the original scene
		cubeShader.use();
		cubeShader.setMat4("view", view);
        cubeShader.setMat4("projection", projection);
        cubeShader.setVec3("lightPos", lightPos);
        cubeShader.setVec3("viewPos", camera.Position);
        cubeShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);			

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, planeTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		model = glm::translate(glm::mat4(), glm::vec3(10, 0, -10));
		cubeShader.setMat4("model", model);
		cubeShader.setInt("object", 1);
		glBindVertexArray(planeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		// Render maze
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cubeTexture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, normalMap);

		cubeShader.setInt("object", 2);
		ourMaze->DrawMaze(cubeShader, cubeVAO);
		RenderWall(cubeShader, cubeVAO);
		if (mazeMap == false) {
			ourMaze->DrawMap(camera.Position.x, camera.Position.z);
		}

		// Render flag
		clothShader.use();
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		model = glm::translate(glm::mat4(), glm::vec3(0.1f, 0.4f, 0.0f)+exitPos);
		model = glm::scale(model, glm::vec3(0.02f, 0.016f, 0.016f));
	    clothShader.setMat4("view", view);
	    clothShader.setMat4("projection", projection);
	    clothShader.setMat4("model", model);
	    clothShader.setVec3("lightPos", lightPos);
        clothShader.setVec3("viewPos", camera.Position);
        cloth.draw();

        modelShader.use();
        model = glm::translate(glm::mat4(), glm::vec3(0.0f, -0.5f, 0.0f)+exitPos);
        model = glm::scale(model, glm::vec3(0.06f, 0.06f, 0.06f));
        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);
        modelShader.setMat4("model", model);
        modelShader.setVec3("lightPos", lightPos);
        modelShader.setVec3("viewPos", camera.Position);
        post.Draw(modelShader);

		model = glm::translate(glm::mat4(), glm::vec3(0.0f, -0.25f, 1.0f));
		model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
		modelShader.setMat4("model", model);
		statue.Draw(modelShader);
		model = glm::translate(model, glm::vec3(10.0f, 0.0f, 0.0f));
		modelShader.setMat4("model", model);
		statue.Draw(modelShader);

		// 树木模型
		for (int i = 0; i <27; i++) {
			model = glm::translate(glm::mat4(), treePos[i]);
			model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
			modelShader.setMat4("model", model);
			tree.Draw(modelShader);
		}
		// 灌木模型
		model = glm::translate(glm::mat4(), glm::vec3(-2.0f, -0.5f, 0.2f));
		model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
		modelShader.setMat4("model", model);
		palm.Draw(modelShader);

		// Render reflection
		glStencilFunc(GL_EQUAL, 10, 0xFF);
        glStencilMask(0x00);
        model = glm::translate(glm::mat4(), glm::vec3(-3.0f, -0.1f, 2.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        modelShader.setMat4("model", model);
        glDepthFunc(GL_ALWAYS);
        tree.Draw(modelShader);
        glDepthFunc(GL_LESS);

        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0x00);

        // Render skeleton animation
		triangle.render(camera); 

		// Render particle
		particleShader.use();
		particleShader.setMat4("view", view);
		particleShader.setMat4("projection", projection);
		glm::vec3 particlePosition(0.0f, 0.0f, 0.0f);
		glm::vec3 velocity(-5.0f, 5.0f, -5.0f);
		Particles.Draw();
		Particles.Update(particleShader, deltaTime, particlePosition, 200, velocity);

		// Render wave
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		wave_model.buildTessendorfWaveMesh(deltaTime);
		wave_model.draw(camera, modelScale, lightPos, movePos, SCR_WIDTH, SCR_HEIGHT);
		glDisable(GL_BLEND);

		// Render skybox
		// change depth function so depth test passes when values are equal to depth buffer's content
		glDepthFunc(GL_LEQUAL);
		skyboxShader.use();
		// remove translation from the view matrix
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);
		skyboxShader.setMat4("model", model);
		// skybox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		// set depth function back to default
		glDepthFunc(GL_LESS);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		RenderText(textShader, "Harry Potter's Maze", 25.0f, 25.0f, 1.0f, glm::vec3(0.502f, 0.502f, 0.251f));
		glDisable(GL_BLEND);

        // now blit multisampled buffer(s) to normal colorbuffer of intermediate FBO. Image is stored in screenTexture
        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);
        glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        // now render quad with scene's visuals as its texture image
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw Screen quad
        screenShader.use();
        glBindVertexArray(quadVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, screenTexture); // use the now resolved color attachment as the quad's texture
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glStencilMask(0xFF);

        // Swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVBO);
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteVertexArrays(1, &planeVBO);
	glDeleteVertexArrays(1, &quadVAO);
	glDeleteBuffers(1, &quadVBO);
	glDeleteVertexArrays(1, &stencilVAO);
	glDeleteBuffers(1, &stencilVBO);

	glfwTerminate();

	delete ourMaze;
	return 0;
}


void RenderScene(Shader& shader, unsigned int& planeVAO, unsigned int& cubeVAO, Maze* ourMaze)
{
	// skybox
	glm::mat4 model;
	model = glm::translate(model, glm::vec3(10, 0, -10));
	shader.setMat4("model", model);
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	ourMaze->DrawMaze(shader, cubeVAO);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window, Maze* ourMaze)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.ProcessKeyboard(FORWARD, deltaTime);
		ourMaze->doCollisions(camera, objectSize, FORWARD, deltaTime);
	}	
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.ProcessKeyboard(BACKWARD, deltaTime);
		ourMaze->doCollisions(camera, objectSize, BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.ProcessKeyboard(LEFT, deltaTime);
		ourMaze->doCollisions(camera, objectSize, LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.ProcessKeyboard(RIGHT, deltaTime);
		ourMaze->doCollisions(camera, objectSize, RIGHT, deltaTime);
	}

	// jump
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		camera.ProcessKeyboard(JUMP, deltaTime);
	}

	// 显示or隐藏小地图
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
		if (M_KEY_ACT) {
			mazeMap = 1 - mazeMap;
			M_KEY_ACT = false;
		} 
	}
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE) {
		M_KEY_ACT = true;
	}
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void cursor_callback(GLFWwindow* window, double xpos, double ypos)
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

void mouse_callback(GLFWwindow* window, int button, int action, int mods)
{
	/**
	 * 
	 *
	 */
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

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
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
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

/**
 * order:
 * +X (right)
 * -X (left)
 * +Y (top)
 * -Y (bottom)
 * +Z (front) 
 * -Z (back)
 */
// ------------------------------------------------
unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	return textureID;
}

void RenderText(Shader &shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
	// Activate corresponding render state	
	shader.use();
	//glUniform3f(glGetUniformLocation(shader.Program, "textColor"), color.x, color.y, color.z);
	shader.setVec3("textColor", glm::vec3(color.x, color.y, color.z));
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(tVAO);

	// Iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = Characters[*c];

		GLfloat xpos = x + ch.Bearing.x * scale;
		GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		GLfloat w = ch.Size.x * scale;
		GLfloat h = ch.Size.y * scale;
		// Update VBO for each character
		GLfloat vertices[6][4] = {
			{ xpos,     ypos + h,   0.0, 0.0 },
		{ xpos,     ypos,       0.0, 1.0 },
		{ xpos + w, ypos,       1.0, 1.0 },

		{ xpos,     ypos + h,   0.0, 0.0 },
		{ xpos + w, ypos,       1.0, 1.0 },
		{ xpos + w, ypos + h,   1.0, 0.0 }
		};
		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, tVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderWall(Shader& shader, unsigned int& cubeVAO) {
	glm::mat4 model;

	glBindVertexArray(cubeVAO);

	model = glm::translate(model, glm::vec3(-3.0f, -0.375, 3.0f));
	model = glm::scale(model, glm::vec3(0.125f, 0.25f, 1.5f));
	shader.setMat4("model", model);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	model = glm::translate(glm::mat4(), glm::vec3(-1.0f, -0.375f, 3.0f));
	model = glm::scale(model, glm::vec3(0.125f, 0.25f, 1.5f));
	shader.setMat4("model", model);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	model = glm::translate(glm::mat4(), glm::vec3(-2.0, -0.375, 3.69f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.125f, 0.25f, 1.9f));
	shader.setMat4("model", model);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	model = glm::translate(glm::mat4(), glm::vec3(-2.0, -0.375, 2.31f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.125f, 0.25f, 1.9f));
	shader.setMat4("model", model);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	glBindVertexArray(0);
}