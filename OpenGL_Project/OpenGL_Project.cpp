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
#include "glm/gtc/type_ptr.hpp"
#include "TreeCluster.hpp"

#pragma region LIGHT STRUCT
#pragma endregion LIGHT STRUCT

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

//uniforms
//matrix uniforms
glm::mat4 model = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);
glm::mat3 normalMatrix = glm::mat3(1.0f);
glm::mat3 lightDirMatrix = glm::mat3(1.0f);
//vector uniforms
glm::vec3 lightDir = glm::vec3(0.0f);
glm::vec3 lightColor = glm::vec3(0.0f);

//angle of directional light
GLfloat lightAngle = 19.5f;

std::vector<const GLchar*> skyboxFaces;

//camera object
gps::Camera myCamera = gps::Camera(glm::vec3(0.0f, 0.0f, 2.5f), glm::vec3(0.0f, 0.0f, -10.0f));

//field of view
double fov = 45.0f;

bool pressedKeys[1024];

//true if mouse moves camera
bool cameraControl = true;

//objects
gps::Model3D ground;
gps::Model3D starship;
gps::Model3D habitat;
gps::Model3D house;
gps::TreeCluster trees;

//skybox
gps::SkyBox skybox;

//shaders
gps::Shader objectShader;
gps::Shader skyboxShader;
gps::Shader depthMapShader;

//shadow FBO and texture
GLuint depthMapFBO;
GLuint depthMapTexture;

#pragma endregion 


#pragma region ERROR_CHECK
// ReSharper disable CppParameterMayBeConst
GLenum glCheckError_(const char *file, int line)
// ReSharper restore CppParameterMayBeConst
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		default: break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)
#pragma endregion 



#pragma region CALLBACKS
// ReSharper disable CppParameterMayBeConst
void windowResizeCallback(GLFWwindow* window, int width, int height)
// ReSharper restore CppParameterMayBeConst
{
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	//set projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), float(retina_width) / float(retina_height), 0.1f, 1000.0f);

	//send matrix data to shader
	objectShader.setMat4("projection", projection);

	//set Viewport transform
	glViewport(0, 0, retina_width, retina_height);
}

// ReSharper disable CppParameterMayBeConst
void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
// ReSharper restore CppParameterMayBeConst
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

	if (key == GLFW_KEY_C && action == GLFW_PRESS) {
		cameraControl = !cameraControl;
		glfwSetInputMode(glWindow, GLFW_CURSOR, cameraControl ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			pressedKeys[key] = true;
		}else if (action == GLFW_RELEASE)
		{
			pressedKeys[key] = false;
		}
	}

//	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
//		refracting = !refracting;
//	}
}

bool firstMouse = true;
double lastX = 0, lastY = 0;
double yaw = 0, pitch = 0;
// ReSharper disable CppParameterMayBeConst
void mouseCallback(GLFWwindow* window, double xpos, double ypos)
// ReSharper restore CppParameterMayBeConst
{
	if (!cameraControl)
		return;

	if (firstMouse) {
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

	if (pitch > 89.0f) {
		pitch = 89.0f;
	}
	if (pitch < -89.0f) {
		pitch = -89.0f;
	}

	myCamera.rotate(pitch, yaw);
}

// ReSharper disable CppParameterMayBeConst
void scrollCallback(GLFWwindow *window, double xoffset, double yoffset)
// ReSharper restore CppParameterMayBeConst
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
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	//for Mac OS X
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", nullptr, nullptr);
	if (!glWindow) {
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

	//glEnable(GL_BLEND);//enable color blending
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);//specify factors

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
	//nanoSuit = gps::Model3D("objects/nanosuit/nanosuit.obj", "objects/nanosuit/");

	//teapot = gps::Model3D("objects/teapots/teapot10segU.obj", "objects/teapots/");

	ground = gps::Model3D("objects/ground/ground.obj", "objects/ground/");

	//lightCube = gps::Model3D("objects/cube/cube.obj", "objects/cube/");

	starship = gps::Model3D("objects/starship/starship.obj", "objects/starship/");

	habitat = gps::Model3D("objects/habitat/Habitat49_update.obj", "objects/habitat/");

	house = gps::Model3D("objects/house/house.obj", "objects/house/");

	trees = gps::TreeCluster("objects/tree/tree.obj", "objects/tree/", 30);

	trees.translate(glm::vec3(0.0f, -1.0f, 0.0f));

	trees.randomize();

	skybox.Load(skyboxFaces);

}

void initShaders()
{
	objectShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");

	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");

	//teapotShader.loadShader("shaders/teapotShader.vert", "shaders/teapotShader.frag");

	//lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");

	depthMapShader.loadShader("shaders/simpleDepthMap.vert", "shaders/simpleDepthMap.frag");
}

void initUniforms() {
	objectShader.useShaderProgram();

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 5.0f, 5.0f);

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light

	glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
	//send the light direction
	objectShader.setVec3("lightDir", lightDirTr);
	//send the light color
	objectShader.setVec3("lightColor", lightColor);

	glCheckError();
}

#pragma endregion 


//TODO
#pragma  region RENDERING

float deltaTime = 0.0f;
float lastTime = 0.0f;
float unitCameraSpeed = 2.0f;

float starshipRotationAngle = 0.0f;

void processMovement()
{
	//get the current time to find time between frames
	float currentTime = glfwGetTime();
	deltaTime = currentTime - lastTime;
	lastTime = currentTime;

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
	
	if (pressedKeys[GLFW_KEY_Q])
	{
		lightAngle += 10.0f * deltaTime;
		if (lightAngle > 360.0f)
		{
			lightAngle = 0.0f;
		}
	}

	if (pressedKeys[GLFW_KEY_E])
	{
		lightAngle -= 10.0f * deltaTime;
		if (lightAngle < 0.0f)
		{
			lightAngle = 360.0f;
		}
	}

	starshipRotationAngle -= 20.0f * deltaTime;
	//lightAngle += 10.0f * deltaTime;
}

glm::mat4 computeLightSpaceTrMatrix()
{
	//create projection matrix
	const GLfloat near_plane = 1.0f, far_plane = 7.5f;
	glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);

	//rotate light to position
	glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
	//create view matrix
	glm::mat4 lightView = glm::lookAt(lightDirTr, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	return lightProjection * lightView;
}


glm::mat4 calculateStarshipModelMatrix()
{
	glm::mat4 m = glm::mat4(1.0f);
	m = glm::rotate(glm::mat4(1.0f), glm::radians(starshipRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	m = glm::translate(m, glm::vec3(50.0f, 5.0f, 0.0f));
	m = glm::scale(m, glm::vec3(0.02f, 0.02f, 0.02f));
	m = glm::rotate(m, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	return m;
}

glm::mat4 calculateGroundModelMatrix()
{
	glm::mat4 m;
	//m = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	//m = glm::scale(m, glm::vec3(6.0f, 0.0f, 6.0f));
	return m;
}

glm::mat4 calculateHabitatModelMatrix()
{
	glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 0.0f));
	return m;
}

glm::mat4 calculateHouseModelMatrix()
{
	glm::mat4 m = glm::mat4(1.0f);
	m = glm::translate(m, glm::vec3(20.0f, -1.0f, 10.0f));
	m = glm::rotate(m, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	return m;
}

void renderToDepthMap()
{
	//configure to use depth map shader
	depthMapShader.useShaderProgram();
	//set viewport to accomodate depth texture
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

	glCullFace(GL_FRONT);
	//bind frameBuffer object
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	//clear depth buffer
	glClear(GL_DEPTH_BUFFER_BIT);

	//calculate and send light space matrix
	depthMapShader.setMat4("lightSpaceTrMatrix", computeLightSpaceTrMatrix());
	trees.draw(depthMapShader, view);

	/*

	//create model matrix for starship
	model = calculateStarshipModelMatrix();
	//send model matrix for starship to shader
	depthMapShader.setMat4("model", model);

	//draw starship
	starship.Draw(depthMapShader);

	//calculat model matrix for habitat
	model = calculateHabitatModelMatrix(); 
	//send model matrix for habitat to shader
	depthMapShader.setMat4("model", model);
	//draw habitat
	habitat.Draw(depthMapShader);

	*/

	//create model matrix for house
	model = calculateHouseModelMatrix();
	depthMapShader.setMat4("model", model);
	//draw house;
	house.Draw(depthMapShader);

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
	//configure to use skybox shader
	skyboxShader.useShaderProgram();
	//draw skybox
	skybox.Draw(skyboxShader, view, projection);
}

void renderObjects()
{

	//configure to use object shader
	objectShader.useShaderProgram();
	//send view matrix data to shader
	objectShader.setMat4("view", view);
	//send projection matrix to shader
	objectShader.setMat4("projection", projection);
	glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
	//send the light direction
	objectShader.setVec3("lightDir", lightDirTr);
	//send lightSpace matrix to shader
	objectShader.setMat4("lightSpaceTrMatrix", computeLightSpaceTrMatrix());
	//send light direction matrix to shader
	objectShader.setMat3("lightDirMatrix", lightDirMatrix);


	//bind depth map
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	//send texture to shader
	objectShader.setInt("shadowMap", 3);

	trees.draw(objectShader, view);

	/*

	//create model matrix for starship
	model = calculateStarshipModelMatrix();
	//send model matrix for starship to shader
	objectShader.setMat4("model", model);
	//create normal matrix for starship
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	//send normal matrix data to shader
	objectShader.setMat3("normalMatrix", normalMatrix);
	//draw starship
	starship.Draw(objectShader);

	//create model matrix for habitat
	model = calculateHabitatModelMatrix();
	//send model matrix for habitat to shader
	objectShader.setMat4("model", model);
	//create normal matrix for habitat
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	//send normal matrix data to shader
	objectShader.setMat3("normalMatrix", normalMatrix);
	//draw habitat
	habitat.Draw(objectShader);

	*/

	//create model matrix for house
	model = calculateHouseModelMatrix();
	objectShader.setMat4("model", model);
	//create normal matrix for house
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	objectShader.setMat3("normalMatrix", normalMatrix);
	//draw house
	house.Draw(objectShader);

	//create model matrix for ground
	model = calculateGroundModelMatrix();
	objectShader.setMat4("model", model);
	//create normal matrix for ground
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	objectShader.setMat3("normalMatrix", normalMatrix);
	//draw ground
	ground.Draw(objectShader);
}

void renderScene()
{
	processMovement();

	projection = glm::perspective((float)glm::radians(fov), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	view = myCamera.getViewMatrix();
	lightDirMatrix = glm::mat3(glm::inverseTranspose(view));

	//set culling to front face
	glCullFace(GL_FRONT);
	
	//first pass, render to depth map to create shadow texture
	renderToDepthMap();
	
	//set culling back to back face
	glCullFace(GL_BACK);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glViewport(0, 0, retina_width, retina_height);

	//render sky box
	renderSkyBox();


	//second pass, render scene
	renderObjects();
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
