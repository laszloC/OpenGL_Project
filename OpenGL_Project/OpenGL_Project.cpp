// OpenGL_Project.cpp : Defines the entry point for the console application.
//

#define GLEW_STATIC
#define TINYOBJLOADER_IMPLEMENTATION

#include "Model3D.hpp"
#include "Camera.hpp"
#include "Shader.hpp"
#include "SkyBox.hpp"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "string"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "TreeCluster.hpp"
#include "Windmill.hpp"

#pragma region LIGHT STRUCT
struct DirLight
{
	glm::vec3 direction;
	glm::vec3 color;

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
};

struct PointLight
{
	glm::vec3 position;
	glm::vec3 color;

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

	float constant;
	float linear;
	float quadratic;
};

#pragma endregion

#pragma region GLOBAL VARIABLES
//window size
int glWindowWidth = 1024;
int glWindowHeight = 768;

//retina width and height
int retina_width, retina_height;

//openFL window
GLFWwindow* glWindow = nullptr;

//size of shadow map
const GLuint SHADOW_WIDTH = 8192;
const GLuint SHADOW_HEIGHT = 8192;

#pragma endregion 

#pragma region CONTROL_VARS

//lights
struct DirLight dirLight;
struct PointLight pointLights[2];
//angle of directional light
GLfloat lightAngle = 19.5f;

//matrix uniforms
glm::mat4 model = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);
glm::mat3 normalMatrix = glm::mat3(1.0f);
glm::mat3 lightDirMatrix = glm::mat3(1.0f);
glm::mat4 lightSpaceTrMatrix = glm::mat4(1.0f);


//camera object
gps::Camera myCamera = gps::Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -10.0f));
//true if mouse moves camera
bool cameraControl = true;

bool wireframe = false;

bool fogEnabled = false;
glm::vec3 fogColor = glm::vec3(0.5f, 0.5f, 0.5f);
float fogDensity = 0.05;

//field of view
double fov = 45.0f;

bool pressedKeys[1024];

//objects
gps::Model3D ground;
gps::Model3D house;
gps::Model3D siege;
gps::Model3D chapel;
gps::Model3D alduin;
gps::Model3D catapult;
gps::Model3D lightCube;
gps::TreeCluster trees;
gps::Windmill windmill;

//skybox
gps::SkyBox skybox;
std::vector<const GLchar*> skyboxFaces;

//shaders
gps::Shader objectShader;
gps::Shader skyboxShader;
gps::Shader depthMapShader;
gps::Shader lightShader;

//shadow FBO and texture
GLuint depthMapFBO;
GLuint depthMapTexture;

#pragma endregion

#pragma region ERROR_CHECK
// ReSharper disable CppParameterMayBeConst
GLenum glCheckError_(const char* file, int line)
// ReSharper restore CppParameterMayBeConst
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM: error = "INVALID_ENUM";
			break;
		case GL_INVALID_VALUE: error = "INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION: error = "INVALID_OPERATION";
			break;
		case GL_STACK_OVERFLOW: error = "STACK_OVERFLOW";
			break;
		case GL_STACK_UNDERFLOW: error = "STACK_UNDERFLOW";
			break;
		case GL_OUT_OF_MEMORY: error = "OUT_OF_MEMORY";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION";
			break;
		default: break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}

#define glCheckError() glCheckError_(__FILE__, __LINE__)
#pragma endregion

#pragma region CALLBACKS

void windowResizeCallback(GLFWwindow* window, int width, int height)
{
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	//set projection matrix
	glm::mat4 projection = glm::perspective(float(glm::radians(fov)), float(retina_width) / float(retina_height), 0.1f,
	                                        1000.0f);

	//send matrix data to shader
	objectShader.setMat4("projection", projection);

	//set Viewport transform
	glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}

	//enable/disable mouse cursor
	if (key == GLFW_KEY_C && action == GLFW_PRESS)
	{
		cameraControl = !cameraControl;
		glfwSetInputMode(glWindow, GLFW_CURSOR, cameraControl ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
	}

	//enable/disable filling
	if (key == GLFW_KEY_V && action == GLFW_PRESS)
	{
		wireframe = !wireframe;
		glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
	}

	//enable/disable fog
	if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		fogEnabled = !fogEnabled;

		objectShader.useShaderProgram();
		objectShader.setBool("fogEnabled", fogEnabled);

		skyboxShader.useShaderProgram();
		skyboxShader.setBool("fogEnabled", fogEnabled);

		lightShader.useShaderProgram();
		lightShader.setBool("fogEnabled", fogEnabled);
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			pressedKeys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			pressedKeys[key] = false;
		}
	}

}

bool firstMouse = true;
double lastX = 0, lastY = 0;
double yaw = 0, pitch = 0;

//use mouse to rotate camera
void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (!cameraControl)
		return;

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	const float sensitivity = 0.05f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
	{
		pitch = 89.0f;
	}
	if (pitch < -89.0f)
	{
		pitch = -89.0f;
	}

	myCamera.rotate(pitch, yaw);
}

//change field of view
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (fov >= 1.0f && fov <= 45.0f)
		fov -= yoffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;
	projection = glm::perspective(float(glm::radians(fov)), float(retina_width) / float(retina_height), 0.1f, 1000.0f);
}
#pragma endregion

#pragma region INIT FUNCTIONS
bool initOpenGLWindow()
{
	if (!glfwInit())
	{
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	//for Mac OS X
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "Medieval Times", nullptr, nullptr);
	if (!glWindow)
	{
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwMakeContextCurrent(glWindow);

	glfwWindowHint(GLFW_SAMPLES, 4);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	// ReSharper disable CppPrintfBadFormat
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);
	// ReSharper restore CppPrintfBadFormat

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	glfwSetKeyCallback(glWindow, keyboardCallback);

	glfwSetCursorPosCallback(glWindow, mouseCallback);

	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSetScrollCallback(glWindow, scrollCallback);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing

	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initFBOs()
{
	//generate FBO id
	glGenFramebuffers(1, &depthMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);

	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//wrap - clip to edge
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	//unbind FBO
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initSkyBox()
{
	skyboxFaces.push_back("skybox/TropicalSunnyDay/TropicalSunnyDayRight2048.png");
	skyboxFaces.push_back("skybox/TropicalSunnyDay/TropicalSunnyDayLeft2048.png");
	skyboxFaces.push_back("skybox/TropicalSunnyDay/TropicalSunnyDayUp2048.png");
	skyboxFaces.push_back("skybox/TropicalSunnyDay/TropicalSunnyDayDown2048.png");
	skyboxFaces.push_back("skybox/TropicalSunnyDay/TropicalSunnyDayBack2048.png");
	skyboxFaces.push_back("skybox/TropicalSunnyDay/TropicalSunnyDayFront2048.png");
}

void initModels()
{
	ground = gps::Model3D("objects/ground/new.obj", "objects/ground/");

	house = gps::Model3D("objects/house/house.obj", "objects/house/");

	siege = gps::Model3D("objects/siege/siege.obj", "objects/siege/");

	chapel = gps::Model3D("objects/chapel/chapel.obj", "objects/chapel/");

	alduin = gps::Model3D("objects/alduin/alduin.obj", "objects/alduin/");

	catapult = gps::Model3D("objects/catapult/catapult.obj", "objects/catapult/");

	trees = gps::TreeCluster("objects/tree/tree.obj", "objects/tree/", 30);

	lightCube = gps::Model3D("objects/cube/globe.obj", "objects/cube/");

	trees.translate(glm::vec3(5.0f, -1.0f, 2.0f));
	trees.randomize(10, 10, 0.8f, 1.3f);

	windmill = gps::Windmill("objects/windmill/mill.obj", "objects/windmill/blade.obj", "objects/windmill/");
	windmill.translate(glm::vec3(10.0f, 5.2f, 20.0f));
	windmill.rotate(glm::radians(-145.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	skybox.Load(skyboxFaces);
}

void initShaders()
{
	objectShader.loadShader("shaders/shaderMulti.vert", "shaders/shaderMulti.frag");

	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");

	depthMapShader.loadShader("shaders/simpleDepthMap.vert", "shaders/simpleDepthMap.frag");

	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
}

void initUniforms()
{
	objectShader.useShaderProgram();

	//directional light
	dirLight.direction = glm::vec3(0.0f, 1.0f, 2.0f);
	dirLight.color = glm::vec3(1.0f, 1.0f, 1.0f); //white light

	dirLight.ambient = glm::vec3(0.4f);
	dirLight.diffuse = glm::vec3(0.8f);
	dirLight.specular = glm::vec3(1.0f);

	//positional light 0
	pointLights[0].position = glm::vec3(4.0f, 2.0f, -10.0f);
	pointLights[0].color = glm::vec3(1.0f, 1.0f, 1.0f);

	pointLights[0].ambient = glm::vec3(0.2f);
	pointLights[0].diffuse = glm::vec3(1.0f);
	pointLights[0].specular = glm::vec3(1.0f);

	pointLights[0].constant = 1.0f;
	pointLights[0].linear = 0.0045f;
	pointLights[0].quadratic = 0.0075f;

	//posiitional light 1
	pointLights[1].position = glm::vec3(20.0f, 2.0f, 3.0f);
	pointLights[1].color = glm::vec3(0.0f, 8.0f, 0.0f);	

	pointLights[1].ambient = glm::vec3(0.2f);
	pointLights[1].diffuse = glm::vec3(0.5f);
	pointLights[1].specular = glm::vec3(0.5f);

	pointLights[1].constant = 1.0f;
	pointLights[1].linear = 0.35f;
	pointLights[1].quadratic = 0.44f;


	glm::vec3 lightDirTr = glm::vec3(
		glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(
			dirLight.direction, 1.0f));

	objectShader.setVec3("dirLight.direction", lightDirTr);
	objectShader.setVec3("dirLight.color", dirLight.color);

	objectShader.setVec3("dirLight.ambient", dirLight.ambient);
	objectShader.setVec3("dirLight.diffuse", dirLight.diffuse);
	objectShader.setVec3("dirLight.specular", dirLight.specular);

	objectShader.setVec3("pointLights[0].position", pointLights[0].position);
	objectShader.setVec3("pointLights[0].color", pointLights[0].color);

	objectShader.setVec3("pointLights[0].ambient", pointLights[0].ambient);
	objectShader.setVec3("pointLights[0].diffuse", pointLights[0].diffuse);
	objectShader.setVec3("pointLights[0].specular", pointLights[0].specular);

	objectShader.setFloat("pointLights[0].constant", pointLights[0].constant);
	objectShader.setFloat("pointLights[0].linear", pointLights[0].linear);
	objectShader.setFloat("pointLights[0].quadratic", pointLights[0].quadratic);
	
	
	objectShader.setVec3("pointLights[1].position", pointLights[1].position);
	objectShader.setVec3("pointLights[1].color", pointLights[1].color);

	objectShader.setVec3("pointLights[1].ambient", pointLights[1].ambient);
	objectShader.setVec3("pointLights[1].diffuse", pointLights[1].diffuse);
	objectShader.setVec3("pointLights[1].specular", pointLights[1].specular);

	objectShader.setFloat("pointLights[1].constant", pointLights[1].constant);
	objectShader.setFloat("pointLights[1].linear", pointLights[1].linear);
	objectShader.setFloat("pointLights[1].quadratic", pointLights[1].quadratic);

	objectShader.setBool("fogEnabled", fogEnabled);
	objectShader.setVec3("fogColor", fogColor);
	objectShader.setFloat("fogDensity", fogDensity);

	skyboxShader.useShaderProgram();
	skyboxShader.setBool("fogEnabled", fogEnabled);
	skyboxShader.setVec3("fogColor", fogColor);

	lightShader.useShaderProgram();
	lightShader.setBool("fogEnabled", fogEnabled);
	lightShader.setVec3("fogColor", fogColor);
	lightShader.setFloat("fogDensity", fogDensity);

	glCheckError();
}

#pragma endregion

#pragma  region RENDERING

float deltaTime = 0.0f;
float lastTime = 0.0f;
float unitCameraSpeed = 2.0f;

float alduinRotationAngle = 0.0f;
float alduinRotationStep = 10.0f;
float currentSin = 0.0f;

void processMovement()
{
	//get the current time to find time between frames
	float currentTime = glfwGetTime();
	deltaTime = currentTime - lastTime;
	lastTime = currentTime;

	//camera movement
	float cameraSpeed = unitCameraSpeed * deltaTime;
	if (pressedKeys[GLFW_KEY_W])
	{
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_S])
	{
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A])
	{
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D])
	{
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}

	//rotate directional light
	if (pressedKeys[GLFW_KEY_Q])
	{
		lightAngle += 10.0f * deltaTime;
		if (lightAngle > 360.0f)
		{
			lightAngle = 0.0f;
		}
		glm::vec3 lightDirTr = glm::vec3(
			glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(
				dirLight.direction, 1.0f));
		//send the light direction
		objectShader.useShaderProgram();
		objectShader.setVec3("dirLight.direction", lightDirTr);
	}

	//rotate directional light
	if (pressedKeys[GLFW_KEY_E])
	{
		lightAngle -= 10.0f * deltaTime;
		if (lightAngle < 0.0f)
		{
			lightAngle = 360.0f;
		}
		glm::vec3 lightDirTr = glm::vec3(
			glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(
				dirLight.direction, 1.0f));
		//send the light direction
		objectShader.useShaderProgram();
		objectShader.setVec3("dirLight.direction", lightDirTr);
	}

	//change red channel
	if (pressedKeys[GLFW_KEY_R])
	{
		if (pressedKeys[GLFW_KEY_UP])
		{
			dirLight.color.r = glm::min(dirLight.color.r + 0.05f, 1.0f);
		}
		if (pressedKeys[GLFW_KEY_DOWN])
		{
			dirLight.color.r = glm::max(dirLight.color.r - 0.05f, 0.0f);
		}
		objectShader.useShaderProgram();
		objectShader.setVec3("dirLight.color", dirLight.color);
	}

	//change green channel
	if (pressedKeys[GLFW_KEY_G])
	{
		if (pressedKeys[GLFW_KEY_UP])
		{
			dirLight.color.g = glm::min(dirLight.color.g + 0.05f, 1.0f);
		}
		if (pressedKeys[GLFW_KEY_DOWN])
		{
			dirLight.color.g = glm::max(dirLight.color.g - 0.05f, 0.0f);
		}
		objectShader.useShaderProgram();
		objectShader.setVec3("dirLight.color", dirLight.color);
	}

	//change blue channel
	if (pressedKeys[GLFW_KEY_B])
	{
		if (pressedKeys[GLFW_KEY_UP])
		{
			dirLight.color.b = glm::min(dirLight.color.b + 0.05f, 1.0f);
		}
		if (pressedKeys[GLFW_KEY_DOWN])
		{
			dirLight.color.b = glm::max(dirLight.color.b - 0.05f, 0.0f);
		}
		objectShader.useShaderProgram();
		objectShader.setVec3("dirLight.color", dirLight.color);
	}

	//increase fog intensity
	if (pressedKeys[GLFW_KEY_KP_ADD])
	{
		fogDensity = glm::min(fogDensity + 0.005f, 0.8f);

		objectShader.useShaderProgram();
		objectShader.setFloat("fogDensity", fogDensity);

		lightShader.useShaderProgram();
		lightShader.setFloat("fogDensity", fogDensity);
	}

	//decrease fog intensity
	if (pressedKeys[GLFW_KEY_KP_SUBTRACT])
	{
		fogDensity = glm::max(fogDensity - 0.005f, 0.05f);

		objectShader.useShaderProgram();
		objectShader.setFloat("fogDensity", fogDensity);

		lightShader.useShaderProgram();
		lightShader.setFloat("fogDensity", fogDensity);
	}

	windmill.rotateBlades(deltaTime);

	alduinRotationAngle += alduinRotationStep * deltaTime;
	if (alduinRotationAngle > 360.0f)
	{
		alduinRotationAngle = 0.0f;
	}
	currentSin = glm::sin(0.5 * alduinRotationAngle);
}

glm::mat4 computeLightSpaceTrMatrix()
{
	//create projection matrix
	const GLfloat near_plane = -100.0f, far_plane = 100.0f;
	glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, near_plane, far_plane);

	//rotate light to position
	glm::vec3 lightDirTr = glm::vec3(
		glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(
			dirLight.direction, 1.0f));
	//create view matrix
	glm::mat4 lightView = glm::lookAt(lightDirTr, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	return lightProjection * lightView;
}

glm::mat4 calculateGroundModelMatrix()
{
	glm::mat4 m = glm::mat4(1.0f);
	return m;
}

glm::mat4 calculateHouseModelMatrix()
{
	glm::mat4 m = glm::mat4(1.0f);
	m = glm::translate(m, glm::vec3(-20.0f, -1.0f, -10.0f));
	m = glm::rotate(m, glm::radians(-110.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	return m;
}

glm::mat4 calculateSiegeModelMatrix()
{
	glm::mat4 m = glm::mat4(1.0f);
	m = glm::translate(m, glm::vec3(15.0f, -1.0f, 10.0f));
	m = glm::rotate(m, glm::radians(-120.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	return m;
}

glm::mat4 calculateChapelModelMatrix()
{
	glm::mat4 m = glm::mat4(1.0f);
	m = glm::translate(m, glm::vec3(-5.0f, -1.0f, -15.0f));
	return m;
}

glm::mat4 calculateAlduinModelMatrix()
{
	glm::mat4 m = glm::mat4(1.0f);
	m = glm::rotate(m, glm::radians(alduinRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	m = glm::translate(m, glm::vec3(20.0f, 15.0f + currentSin, 0.0f));
	m = glm::rotate(m, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	//m = glm::rotate(m, glm::radians(.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	return m;
}

glm::mat4 calculateCatapultModelMatrix()
{
	glm::mat4 m = glm::mat4(1.0f);
	m = glm::translate(m, glm::vec3(20.0f, -1.0f, -10.0f));
	m = glm::rotate(m, glm::radians(230.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	return m;
}

void renderToDepthMap()
{
	glCullFace(GL_FRONT);
	//configure to use depth map shader
	depthMapShader.useShaderProgram();
	//set viewport to accomodate depth texture
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

	//bind frameBuffer object
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	//clear depth buffer
	glClear(GL_DEPTH_BUFFER_BIT);

	//calculate and send light space matrix
	depthMapShader.setMat4("lightSpaceTrMatrix", lightSpaceTrMatrix);
	trees.draw(depthMapShader, view);


	windmill.draw(depthMapShader, view);

	//create model matrix for house
	model = calculateHouseModelMatrix();
	depthMapShader.setMat4("model", model);
	//draw house;
	house.Draw(depthMapShader);

	//create model matrix for siege
	model = calculateSiegeModelMatrix();
	depthMapShader.setMat4("model", model);
	//draw siege
	siege.Draw(depthMapShader);

	//create model matrix for chapel
	model = calculateChapelModelMatrix();
	depthMapShader.setMat4("model", model);
	//draw chapel
	chapel.Draw(depthMapShader);

	//create model matrix for alduin
	model = calculateAlduinModelMatrix();
	depthMapShader.setMat4("model", model);
	//draw alduin
	alduin.Draw(depthMapShader);

	//create model matrix for catapult
	model = calculateCatapultModelMatrix();
	depthMapShader.setMat4("model", model);
	//dreaw catapult
	catapult.Draw(depthMapShader);

	//create model matrix for ground
	model = calculateGroundModelMatrix();
	depthMapShader.setMat4("model", model);
	//draw ground
	ground.Draw(depthMapShader);

	//unbind frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderSkyBox()
{
	glCullFace(GL_BACK);
	//configure to use skybox shader
	skyboxShader.useShaderProgram();
	//draw skybox
	skybox.Draw(skyboxShader, view, projection);
}

void renderObjects()
{
	glCullFace(GL_BACK);
	//configure to use object shader
	objectShader.useShaderProgram();
	//send view matrix data to shader
	view = myCamera.getViewMatrix();
	objectShader.setMat4("view", view);
	//send projection matrix to shader
	projection = glm::perspective(float(glm::radians(fov)), float(retina_width) / float(retina_height), 0.1f, 1000.0f);
	objectShader.setMat4("projection", projection);
	glm::vec3 lightDirTr = glm::vec3(
		glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(
			dirLight.direction, 1.0f));
	//send the light direction
	objectShader.setVec3("dirLight.direction", lightDirTr);
	//send lightSpace matrix to shader
	objectShader.setMat4("lightSpaceTrMatrix", lightSpaceTrMatrix);
	lightDirMatrix = glm::mat3(glm::inverseTranspose(view));
	//send light direction matrix to shader
	objectShader.setMat3("lightDirMatrix", lightDirMatrix);
	
//	glm::vec3 lightPos0Eye = glm::vec3(view * glm::vec4(pointLights[0].position, 1.0f));
//	objectShader.setVec3("pointLights[0].position", lightPos0Eye);

	//bind depth map
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	//send texture to shader
	objectShader.setInt("shadowMap", 3);

	trees.draw(objectShader, view);

	windmill.draw(objectShader, view);

	//create model matrix for house
	model = calculateHouseModelMatrix();
	objectShader.setMat4("model", model);
	//create normal matrix for house
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	objectShader.setMat3("normalMatrix", normalMatrix);
	//draw house
	house.Draw(objectShader);

	//create model matrix for siege
	model = calculateSiegeModelMatrix();
	objectShader.setMat4("model", model);
	//create normal matrix for siege
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	objectShader.setMat3("normalMatrix", normalMatrix);
	//draw siege
	siege.Draw(objectShader);

	//create model matrix for chapel
	model = calculateChapelModelMatrix();
	objectShader.setMat4("model", model);
	//create normal matrix for chapel
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	objectShader.setMat3("normalMatrix", normalMatrix);
	//draw chapel
	chapel.Draw(objectShader);

	//create model matrix for alduin
	model = calculateAlduinModelMatrix();
	objectShader.setMat4("model", model);
	//create normal matrix for alduin
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	objectShader.setMat3("normalMatrix", normalMatrix);
	//draw alduin
	alduin.Draw(objectShader);

	//create model matrix for catapult
	model = calculateCatapultModelMatrix();
	objectShader.setMat4("model", model);
	//create normal matrix for catapult
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	objectShader.setMat3("normalMatrix", normalMatrix);
	//draw catapult
	catapult.Draw(objectShader);

	//create model matrix for ground
	model = calculateGroundModelMatrix();
	objectShader.setMat4("model", model);
	//create normal matrix for ground
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	objectShader.setMat3("normalMatrix", normalMatrix);
	//draw ground
	ground.Draw(objectShader);
}

void renderCube(glm::vec3 pos, glm::vec3 color)
{
	glCullFace(GL_BACK);
	//draw a white cube around the light

	projection = glm::perspective((float)glm::radians(fov), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	view = myCamera.getViewMatrix();

	lightShader.useShaderProgram();
	lightShader.setMat4("view", view);
	lightShader.setMat4("projection", projection);
	lightShader.setVec3("color", color);
	//model = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(glm::mat4(1.0f), pos);
	model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
	lightShader.setMat4("model", model);

	lightCube.Draw(lightShader);
}

void renderScene()
{
	processMovement();

	projection = glm::perspective((float)glm::radians(fov), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	view = myCamera.getViewMatrix();

	lightDirMatrix = glm::mat3(glm::inverseTranspose(view));
	lightSpaceTrMatrix = computeLightSpaceTrMatrix();

	//set culling to front face
	//	glCullFace(GL_BACK);

	//first pass, render to depth map to create shadow texture
	renderToDepthMap();

	//set culling back to back face
	//	glCullFace(GL_BACK);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, retina_width, retina_height);

	//render sky box
	renderSkyBox();

	//second pass, render scene
	renderObjects();

	//render light cube(s)
	renderCube(pointLights[0].position, pointLights[0].color);
	renderCube(pointLights[1].position, pointLights[1].color);
}

#pragma endregion

#pragma region ENTRY POINT
int main()
{
	initOpenGLWindow();
	initOpenGLState();
	initFBOs();
	initSkyBox();
	initModels();
	initShaders();
	initUniforms();

	glCheckError();

	while (!glfwWindowShouldClose(glWindow))
	{
		renderScene();
		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	glfwTerminate();

	return 0;
}
#pragma endregion
