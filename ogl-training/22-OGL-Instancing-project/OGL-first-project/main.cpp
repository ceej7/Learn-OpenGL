#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include "Shader.h"
#include "stb_image.h"
#include "camera.h"
#include "Model.h"
using namespace::std;
// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------

//float quadVertices[] = {
//	// positions     // colors
//	-0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
//	 0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
//	-0.05f, -0.05f,  0.0f, 0.0f, 1.0f,
//
//	-0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
//	 0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
//	 0.05f,  0.05f,  0.0f, 1.0f, 1.0f
//};
//camera
Camera camera(glm::vec3(0.0f, 0.0f, 155.0f));

//framerate
float currentFrame = glfwGetTime();
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = currentFrame;

//screen parameter
int screenWidth = 1920;
int screenHeight = 1080;
float lastX = screenWidth/2, lastY = screenHeight/2;
bool firstMouse = true;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(char const *path);
int main()
{
	//------------------------------------Window initial
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	glViewport(0, 0, screenWidth, screenHeight);

	//------------------------------------register
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	
	//------------------------------------shader
	Shader normalShader("shader/normalshader.vs", NULL, "shader/normalshader.fs");
	Shader instanceShader("shader/instanceShader.vs", NULL, "shader/instanceShader.fs");

	//------------------------------------z-buffer
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	////object
	//glm::vec2 translations[100];
	//int index = 0;
	//float offset = 0.1f;
	//for (int y = -10; y < 10; y += 2)
	//{
	//	for (int x = -10; x < 10; x += 2)
	//	{
	//		glm::vec2 translation;
	//		translation.x = (float)x / 10.0f + offset;
	//		translation.y = (float)y / 10.0f + offset;
	//		translations[index++] = translation;
	//	}
	//}
	//unsigned int pointVAO, pointVBO,offsetVBO;
	//glGenVertexArrays(1, &pointVAO);
	//glBindVertexArray(pointVAO);
	//glGenBuffers(1, &pointVBO);
	//glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
	//glEnableVertexAttribArray(1);
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(2*sizeof(float)));
	//glGenBuffers(1, &offsetVBO);
	//glBindBuffer(GL_ARRAY_BUFFER, offsetVBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 100, &translations[0], GL_STATIC_DRAW);
	//glEnableVertexAttribArray(2);
	//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glVertexAttribDivisor(2, 1);
	//glBindVertexArray(0);
	//model
	char path_Rock[] = "./rock/rock.obj";
	Model rock(path_Rock);
	char path_Planet[] = "./planet/planet.obj";
	Model planet(path_Planet);
	unsigned int amount = 10000;
	glm::mat4 *modelMatrices;
	modelMatrices = new glm::mat4[amount];
	srand(glfwGetTime()); // initialize random seed	
	float radius = 80.0;
	float offset = 20.0f;
	for (unsigned int i = 0; i < amount; i++)
	{
		glm::mat4 model = glm::mat4(1.0f);
		// 1. translation: displace along circle with 'radius' in range [-offset, offset]
		float angle = (float)i / (float)amount * 360.0f;
		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float x = sin(angle) * radius + displacement;
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float y = displacement * 0.4f; // keep height of field smaller compared to width of x and z
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float z = cos(angle) * radius + displacement;
		model = glm::translate(model, glm::vec3(x, y, z));

		// 2. scale: Scale between 0.05 and 0.25f
		float scale = (rand() % 20) / 100.0f + 0.05;
		model = glm::scale(model, glm::vec3(scale));

		// 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
		float rotAngle = (rand() % 360);
		model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

		// 4. now add to list of matrices
		modelMatrices[i] = model;
	}
	unsigned int buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

	for (unsigned int i = 0; i < rock.meshes.size(); i++)
	{
		unsigned int VAO = rock.meshes[i].VAO;
		glBindVertexArray(VAO);
		// vertex Attributes
		GLsizei vec4Size = sizeof(glm::vec4);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		glBindVertexArray(0);
	}


	//------------------------------------render loop
	while (!glfwWindowShouldClose(window))
	{
		//frametime
		currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		processInput(window);

		// rendering commands here
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);//state-setting
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//state-using

		// planet
		normalShader.use();
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f,-3.0f,0.0f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(4.0f, 4.0f, 4.0f));
		//view
		glm::mat4 viewMatrix = camera.GetViewMatrix();
		//proj
		glm::mat4 projectionMatrix;
		projectionMatrix = glm::perspective(glm::radians(camera.Zoom), (float)screenWidth / (float)screenHeight, 0.1f, 1000.0f);
		normalShader.setMat4("model", modelMatrix);
		normalShader.setMat4("view", viewMatrix);
		normalShader.setMat4("projection", projectionMatrix);
		planet.Draw(normalShader);

		//draw rocks
		//for (unsigned int i = 0; i < amount; i++)
		//{
		//	normalShader.setMat4("model", modelMatrices[i]);
		//	rock.Draw(normalShader);
		//}
		instanceShader.use();
		instanceShader.setMat4("view", viewMatrix);
		instanceShader.setMat4("projection", projectionMatrix);
		for (unsigned int i = 0; i < rock.meshes.size(); i++)
		{
			glBindVertexArray(rock.meshes[i].VAO);
			glDrawElementsInstanced(
				GL_TRIANGLES, rock.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, amount
			);
		}

		
		//normalShader.use();
		//glBindVertexArray(pointVAO);
		//glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 100);

		// check and call events and swap the buffers
		glfwPollEvents();
		glfwSwapBuffers(window);

	}
	glfwTerminate();
	return 0;

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	screenHeight = height;
	screenWidth = width;
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse) // this bool variable is initially set to true
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	else {
		camera.ProcessMouseMovement(xpos - lastX, lastY - ypos);
		lastX = xpos;
		lastY = ypos;

	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	float cameraSpeed = 2.5f * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const *path)
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