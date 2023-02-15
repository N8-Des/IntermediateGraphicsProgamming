#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "EW/Shader.h"
#include "EW/EwMath.h"
#include "EW/Camera.h"
#include "EW/Mesh.h"
#include "EW/Transform.h"
#include "EW/ShapeGen.h"

void processInput(GLFWwindow* window);
void resizeFrameBufferCallback(GLFWwindow* window, int width, int height);
void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods);
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void mousePosCallback(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

float lastFrameTime;
float deltaTime;

int SCREEN_WIDTH = 1080;
int SCREEN_HEIGHT = 720;

double prevMouseX;
double prevMouseY;
bool firstMouseInput = false;

/* Button to lock / unlock mouse
* 1 = right, 2 = middle
* Mouse will start locked. Unlock it to use UI
* */
const int MOUSE_TOGGLE_BUTTON = 1;
const float MOUSE_SENSITIVITY = 0.1f;
const float CAMERA_MOVE_SPEED = 5.0f;
const float CAMERA_ZOOM_SPEED = 3.0f;

Camera camera((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT);

glm::vec3 bgColor = glm::vec3(0);
glm::vec3 lightColor = glm::vec3(1.0f);
glm::vec3 lightPosition = glm::vec3(0.0f, 3.0f, 0.0f);

bool wireFrame = false;

struct DirectionalLight
{
	glm::vec3 color = glm::vec3(1);
	glm::vec3 direction = glm::vec3(1, -1, 0);
	float intensity = 1.0f;
};

struct PointLight
{
	glm::vec3 color = glm::vec3(1);
	glm::vec3 position = glm::vec3(0);
	float attenuation = 1;
	float intensity = 1.0f;
};
struct SpotLight
{
	glm::vec3 color = glm::vec3(1);
	glm::vec3 position = glm::vec3(0, 2, 0);
	glm::vec3 direction = glm::vec3(0, -1, 0);;
	float attenuation = 1;
	float intensity = 1.0f;
	float minAngle = 80;
	float maxAngle = 140;
};
struct Material
{
	glm::vec3 color = glm::vec3(1, 1, 1);
	float ambientK = 0.2f;
	float diffuseK = 0.5f;
	float specularK = 0.5f;
	float shininess = 150.0f;
};
int main() {
	if (!glfwInit()) {
		printf("glfw failed to init");
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Lighting", 0, 0);
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		printf("glew failed to init");
		return 1;
	}

	glfwSetFramebufferSizeCallback(window, resizeFrameBufferCallback);
	glfwSetKeyCallback(window, keyboardCallback);
	glfwSetScrollCallback(window, mouseScrollCallback);
	glfwSetCursorPosCallback(window, mousePosCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);

	//Hide cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Setup UI Platform/Renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	//Dark UI theme.
	ImGui::StyleColorsDark();

	//Used to draw shapes. This is the shader you will be completing.
	Shader litShader("shaders/defaultLit.vert", "shaders/defaultLit.frag");

	//Used to draw light sphere
	Shader unlitShader("shaders/defaultLit.vert", "shaders/unlit.frag");

	ew::MeshData cubeMeshData;
	ew::createCube(1.0f, 1.0f, 1.0f, cubeMeshData);
	ew::MeshData sphereMeshData;
	ew::createSphere(0.5f, 64, sphereMeshData);
	ew::MeshData cylinderMeshData;
	ew::createCylinder(1.0f, 0.5f, 64, cylinderMeshData);
	ew::MeshData planeMeshData;
	ew::createPlane(1.0f, 1.0f, planeMeshData);

	ew::Mesh cubeMesh(&cubeMeshData);
	ew::Mesh sphereMesh(&sphereMeshData);
	ew::Mesh planeMesh(&planeMeshData);
	ew::Mesh cylinderMesh(&cylinderMeshData);

	//Enable back face culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Enable depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//Initialize shape transforms
	ew::Transform cubeTransform;
	ew::Transform sphereTransform;
	ew::Transform planeTransform;
	ew::Transform cylinderTransform;
	ew::Transform pointLight1Transform;
	ew::Transform pointLight2Transform;
	cubeTransform.position = glm::vec3(-2.0f, 0.0f, 0.0f);
	sphereTransform.position = glm::vec3(0.0f, 0.0f, 0.0f);

	planeTransform.position = glm::vec3(0.0f, -1.0f, 0.0f);
	planeTransform.scale = glm::vec3(10.0f);

	cylinderTransform.position = glm::vec3(2.0f, 0.0f, 0.0f);

	pointLight1Transform.scale = glm::vec3(0.5f);
	pointLight2Transform.scale = glm::vec3(0.5f);

	Material mat;
	mat.color = glm::vec3(1, 0, 0);
	DirectionalLight directionLight;
	PointLight pointLight1;
	PointLight pointLight2;
	pointLight1.position = glm::vec3(1, 1, 0);
	pointLight2.position = glm::vec3(-1, 1, 0);
	SpotLight spotlight;
	spotlight.position = glm::vec3(0, 2, 0);
	spotlight.direction = glm::vec3(0, -1, 0);
	while (!glfwWindowShouldClose(window)) {
		processInput(window);
		glClearColor(bgColor.r,bgColor.g,bgColor.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		float time = (float)glfwGetTime();
		deltaTime = time - lastFrameTime;
		lastFrameTime = time;

		//Draw
		litShader.use();

		litShader.setMat4("_Projection", camera.getProjectionMatrix());
		litShader.setMat4("_View", camera.getViewMatrix());

		//directional light
		litShader.setVec3("_DirectionalLight.direction", directionLight.direction);
		litShader.setVec3("_DirectionalLight.color", directionLight.color);
		litShader.setFloat("_DirectionalLight.intensity", directionLight.intensity);

		//point lights
		litShader.setVec3("_PointLights[0].position", pointLight1.position);
		litShader.setVec3("_PointLights[0].color", pointLight1.color);
		litShader.setFloat("_PointLights[0].intensity", pointLight1.intensity);
		litShader.setFloat("_PointLights[0].attenuation", pointLight1.attenuation);

		litShader.setVec3("_PointLights[1].position", pointLight2.position);
		litShader.setVec3("_PointLights[1].color", pointLight2.color);
		litShader.setFloat("_PointLights[1].intensity", pointLight2.intensity);
		litShader.setFloat("_PointLights[1].attenuation", pointLight2.attenuation);

		//spot light
		litShader.setVec3("_SpotLight.position", spotlight.position);
		litShader.setVec3("_SpotLight.direction", spotlight.direction);
		litShader.setVec3("_SpotLight.color", spotlight.color);
		litShader.setFloat("_SpotLight.intensity", spotlight.intensity);
		litShader.setFloat("_SpotLight.attenuation", spotlight.attenuation);
		litShader.setFloat("_SpotLight.minAngle", spotlight.minAngle);
		litShader.setFloat("_SpotLight.maxAngle", spotlight.maxAngle);


		//view position
		litShader.setVec3("_ViewPos", camera.getPosition());
		

		//Materials
		litShader.setVec3("_Material.color", mat.color);
		litShader.setFloat("_Material.ambientK", mat.ambientK);
		litShader.setFloat("_Material.diffuseK", mat.diffuseK);
		litShader.setFloat("_Material.specularK", mat.specularK);
		litShader.setFloat("_Material.shininess", mat.shininess);



		//Draw cube
		litShader.setMat4("_Model", cubeTransform.getModelMatrix());
		cubeMesh.draw();

		//Draw sphere
		litShader.setMat4("_Model", sphereTransform.getModelMatrix());
		sphereMesh.draw();

		//Draw cylinder
		litShader.setMat4("_Model", cylinderTransform.getModelMatrix());
		cylinderMesh.draw();

		//Draw plane
		litShader.setMat4("_Model", planeTransform.getModelMatrix());
		planeMesh.draw();

		unlitShader.use();
		pointLight1Transform.position = pointLight1.position;
		pointLight2Transform.position = pointLight2.position;
		unlitShader.setMat4("_Projection", camera.getProjectionMatrix());
		unlitShader.setMat4("_View", camera.getViewMatrix());
		unlitShader.setMat4("_Model", pointLight1Transform.getModelMatrix());
		unlitShader.setVec3("_Color", pointLight1.color);
		sphereMesh.draw();
		unlitShader.setMat4("_Model", pointLight2Transform.getModelMatrix());
		unlitShader.setVec3("_Color", pointLight2.color);
		sphereMesh.draw();

		//Draw UI
		ImGui::Begin("Settings");

	
		ImGui::SliderFloat("Material Ambient", &mat.ambientK, 0, 1);
		ImGui::SliderFloat("Material Diffuse", &mat.diffuseK, 0, 1);
		ImGui::SliderFloat("Material Specular", &mat.specularK, 0, 1);
		ImGui::SliderFloat("Material Shininess", &mat.shininess, 1, 512);
		ImGui::ColorEdit3("Material Color", &mat.color.r);
		ImGui::End();

		ImGui::Begin("Directional Settings");
		ImGui::SliderFloat("Directional Light Intensity", &directionLight.intensity, 0, 5);
		ImGui::ColorEdit3("Directional Light Color", &directionLight.color.r);
		ImGui::DragFloat3("Directional Light Direction", &directionLight.direction.x);
		ImGui::End();
		ImGui::Begin("Point Settings");
		ImGui::SliderFloat("Point Light 1 Intensity", &pointLight1.intensity, 0, 5);
		ImGui::SliderFloat("Point Light 1 Atten.", &pointLight1.attenuation, 0, 5);
		ImGui::ColorEdit3("Point Light 1 Color", &pointLight1.color.r);
		ImGui::DragFloat3("Point Light 1 Position", &pointLight1.position.x);

		ImGui::SliderFloat("Point Light 2 Intensity", &pointLight2.intensity, 0, 5);
		ImGui::SliderFloat("Point Light 2 Atten.", &pointLight2.attenuation, 0, 5);
		ImGui::ColorEdit3("Point Light 2 Color", &pointLight2.color.r);
		ImGui::DragFloat3("Point Light 2 Position", &pointLight2.position.x);
		ImGui::End();

		ImGui::Begin("Spotlight Settings");
		ImGui::DragFloat3("Spotlight Position", &spotlight.position.x);
		ImGui::DragFloat3("Spotlight Direction", &spotlight.direction.x);

		ImGui::SliderFloat("Spotlight Intensity", &spotlight.intensity, 0, 5);
		ImGui::SliderFloat("Spotlight Atten.", &spotlight.attenuation, 0, 5);
		ImGui::ColorEdit3("Spotlight Color", &spotlight.color.r);
		ImGui::SliderFloat("Spotlight Min Angle", &spotlight.minAngle, 0, 360);
		ImGui::SliderFloat("Spotlight Max Angle", &spotlight.maxAngle, 0, 360);
		ImGui::End();


		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwPollEvents();

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}
//Author: Eric Winebrenner
void resizeFrameBufferCallback(GLFWwindow* window, int width, int height)
{
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
	camera.setAspectRatio((float)SCREEN_WIDTH / SCREEN_HEIGHT);
	glViewport(0, 0, width, height);
}
//Author: Eric Winebrenner
void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods)
{
	if (keycode == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	//Reset camera
	if (keycode == GLFW_KEY_R && action == GLFW_PRESS) {
		camera.setPosition(glm::vec3(0, 0, 5));
		camera.setYaw(-90.0f);
		camera.setPitch(0.0f);
		firstMouseInput = false;
	}
	if (keycode == GLFW_KEY_1 && action == GLFW_PRESS) {
		wireFrame = !wireFrame;
		glPolygonMode(GL_FRONT_AND_BACK, wireFrame ? GL_LINE : GL_FILL);
	}
}
//Author: Eric Winebrenner
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (abs(yoffset) > 0) {
		float fov = camera.getFov() - (float)yoffset * CAMERA_ZOOM_SPEED;
		camera.setFov(fov);
	}
}
//Author: Eric Winebrenner
void mousePosCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
		return;
	}
	if (!firstMouseInput) {
		prevMouseX = xpos;
		prevMouseY = ypos;
		firstMouseInput = true;
	}
	float yaw = camera.getYaw() + (float)(xpos - prevMouseX) * MOUSE_SENSITIVITY;
	camera.setYaw(yaw);
	float pitch = camera.getPitch() - (float)(ypos - prevMouseY) * MOUSE_SENSITIVITY;
	pitch = glm::clamp(pitch, -89.9f, 89.9f);
	camera.setPitch(pitch);
	prevMouseX = xpos;
	prevMouseY = ypos;
}
//Author: Eric Winebrenner
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	//Toggle cursor lock
	if (button == MOUSE_TOGGLE_BUTTON && action == GLFW_PRESS) {
		int inputMode = glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
		glfwSetInputMode(window, GLFW_CURSOR, inputMode);
		glfwGetCursorPos(window, &prevMouseX, &prevMouseY);
	}
}

//Author: Eric Winebrenner
//Returns -1, 0, or 1 depending on keys held
float getAxis(GLFWwindow* window, int positiveKey, int negativeKey) {
	float axis = 0.0f;
	if (glfwGetKey(window, positiveKey)) {
		axis++;
	}
	if (glfwGetKey(window, negativeKey)) {
		axis--;
	}
	return axis;
}

//Author: Eric Winebrenner
//Get input every frame
void processInput(GLFWwindow* window) {

	float moveAmnt = CAMERA_MOVE_SPEED * deltaTime;

	//Get camera vectors
	glm::vec3 forward = camera.getForward();
	glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0,1,0)));
	glm::vec3 up = glm::normalize(glm::cross(forward, right));

	glm::vec3 position = camera.getPosition();
	position += forward * getAxis(window, GLFW_KEY_W, GLFW_KEY_S) * moveAmnt;
	position += right * getAxis(window, GLFW_KEY_D, GLFW_KEY_A) * moveAmnt;
	position += up * getAxis(window, GLFW_KEY_Q, GLFW_KEY_E) * moveAmnt;
	camera.setPosition(position);
}