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
#include "EW/ShapeGen.h"
#include <iostream>

void resizeFrameBufferCallback(GLFWwindow* window, int width, int height);
void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods);

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

glm::vec3 bgColor = glm::vec3(0);
float exampleSliderFloat = 0.0f;
const int CUBE_AMOUNT = 50;

struct CameraSettings
{
	float cameraRadius = 7.0f;
	float cameraSpeed = 1.0f;
	float FOV = 60.0f;
	float orthoSize = 2.0f;
	glm::vec3 position = glm::vec3(0);
	glm::vec3 target = glm::vec3(0);
	bool orthoToggle = false;
	glm::mat4 getViewMatrix()
	{
		glm::vec3 up(0, 1, 0);
		glm::vec3 forward = target - position;
		forward = glm::normalize(forward);

		glm::vec3 right = glm::cross(forward, up);
		right = glm::normalize(right);
		
		up = glm::cross(right, forward);

		forward *= -1;

		glm::mat4 rCam
		{
			right.x, right.y, right.z, 0,
			up.x, up.y, up.z, 0,
			forward.x, forward.y, forward.z, 0,
			0, 0, 0, 1
		};
		rCam = glm::transpose(rCam);

		glm::mat4 viewTranslation
		{
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			-position.x, -position.y, -position.z, 1
		};
		return rCam * viewTranslation;
	};

	glm::mat4 getProjectionMatrix(float aspectRatio, float near, float far)
	{
		glm::mat4 projection
		{
			1 / (aspectRatio * tan(glm::radians(FOV) / 2)), 0, 0, 0,
			0, 1 / tan(glm::radians(FOV) / 2), 0, 0,
			0, 0, -((far + near) / (far - near)), -1,
			0, 0, -((2 * far * near) / (far - near)), 1
		};
		return projection;
	};

	glm::mat4 getOrthoMatrix(float aspectRatio, float height, float near, float far) 
	{
		float r = height * aspectRatio;
		float t = height;
		float l = -r;
		float b = -t;

		glm::mat4 ortho
		{
			2 / (r - l), 0, 0, 0,
			0, 2 / (t - b), 0, 0,
			0, 0, -2 / (far - near), 0,
			- (r + l) / (r - l), -(t + b) / (t - b), -(far + near) / (far - near), 1
		};
		return ortho;
	}
};

struct Transform
{
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
	//IMPORTANT: this is just here for my cool little system to be shown off. It obviously doesnt hold any vital infomration.
	bool increasing = true;

	glm::mat4 getModelMatrix()
	{
		glm::mat4 translationMatrix
		{
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			position.x, position.y, position.z, 1
		};

		glm::mat4 scaleMatrix
		{
			scale.x, 0, 0, 0,
			0, scale.y, 0, 0,
			0, 0, scale.z, 0,
			0, 0, 0, 1
		};

		//ROTATIONS!!
		glm::mat4 rotX
		{
			1, 0, 0, 0,
			0, cos(rotation.x), sin(rotation.x), 0,
			0, -sin(rotation.x), cos(rotation.x), 0,
			0, 0, 0, 1
		};

		glm::mat4 rotY
		{
			cos(rotation.y), 0, -sin(rotation.y), 0,
			0, 1, 0, 0,
			sin(rotation.y), 0, cos(rotation.y), 0,
			0, 0, 0, 1
		};

		glm::mat4 rotZ
		{
			cos(rotation.z), sin(rotation.z), 0, 0,
			-sin(rotation.z), cos(rotation.z), 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		};

		glm::mat4 rotationMatrix = rotY * rotX * rotZ;

		return translationMatrix * rotationMatrix * scaleMatrix;		
	}
};


int main() {
	if (!glfwInit()) {
		printf("glfw failed to init");
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Transformations", 0, 0);
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		printf("glew failed to init");
		return 1;
	}

	glfwSetFramebufferSizeCallback(window, resizeFrameBufferCallback);
	glfwSetKeyCallback(window, keyboardCallback);

	// Setup UI Platform/Renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	//Dark UI theme.
	ImGui::StyleColorsDark();

	Shader shader("shaders/vertexShader.vert", "shaders/fragmentShader.frag");

	MeshData cubeMeshData;
	createCube(1.0f, 1.0f, 1.0f, cubeMeshData);

	Mesh cubeMesh(&cubeMeshData);

	//Enable back face culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Enable depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	CameraSettings camera;
	Transform cubeTransforms[CUBE_AMOUNT];
	for (int i = 0; i < CUBE_AMOUNT; i++)
	{
		cubeTransforms[i].position = glm::vec3((float) - CUBE_AMOUNT * 0.1f + i * 0.2f, 0, 0);
		cubeTransforms[i].rotation = glm::vec3(0, 0, 0);
		cubeTransforms[i].scale = glm::vec3(0.2f, 1, 1);
	}

	while (!glfwWindowShouldClose(window)) {
		glClearColor(bgColor.r,bgColor.g,bgColor.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		float time = (float)glfwGetTime();
		camera.position.z = sin(time * camera.cameraSpeed) * camera.cameraRadius;
		camera.position.x = cos(time * camera.cameraSpeed) * camera.cameraRadius;
		glm::mat4 projection;
		glm::mat4 view = camera.getViewMatrix();
		if (camera.orthoToggle)
		{
			 projection = camera.getOrthoMatrix((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, camera.orthoSize, 0.1f, 100.0f);
		}
		else
		{
			projection = camera.getProjectionMatrix((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
		}
		shader.setFloat("time", time);
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		shader.use();
		for (int i = 0; i < CUBE_AMOUNT; i++)
		{

			if (i * 0.1f < time)
			{
				if (cubeTransforms[i].scale.y >= 6)
				{
					cubeTransforms[i].increasing = false;
				}
				if (cubeTransforms[i].scale.y <= 0.8f)
				{
					cubeTransforms[i].increasing = true;
				}
				if (cubeTransforms[i].increasing)
				{
					cubeTransforms[i].scale.y += 0.05f;
				}
				else
				{
					cubeTransforms[i].scale.y -= 0.07f;
				}
			}
			shader.setMat4("model", cubeTransforms[i].getModelMatrix());
			shader.setFloat("yScale", cubeTransforms[i].scale.y);
			cubeMesh.draw();
		}
		deltaTime = time - lastFrameTime;
		lastFrameTime = time;

		//Draw
		//Draw UI
		ImGui::Begin("Settings");
		ImGui::SliderFloat("Camera Radius", &camera.cameraRadius, 0.0f, 15.0f);
		ImGui::SliderFloat("Camera Speed", &camera.cameraSpeed, 0.0f, 20.0f);
		ImGui::SliderFloat("Ortho Size", &camera.orthoSize, 0.0f, 40.0f);

		ImGui::SliderFloat("Camera FOV", &camera.FOV, 0.0f, 80.0f);
		ImGui::Checkbox("Orthographic Toggle", &camera.orthoToggle);
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwPollEvents();

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

void resizeFrameBufferCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
}

void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods)
{
	if (keycode == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}