#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "ModelVertices.h"
#include "stb_image.h"
#include "camera.h"

// - screen parameters
int screenWidth = 1280;
int screenHeight = 720;
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

// - camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = screenWidth/2, lastY = screenHeight/2;
bool firstMouse = true;

// - framerate
float currentFrame = glfwGetTime();
float deltaTime = 0.0f;
float lastFrame = currentFrame;

// - VAO
unsigned int planeVAO;
//texture
unsigned int planeDiffuse;
unsigned int planeNormal;
unsigned int planeSpecular;
unsigned int boxDiffuse;
unsigned int boxNormal;
unsigned int boxSpecular;

// - functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(char const * path);
void renderQuad();
void renderCube();
void RenderScene(Shader& shader, glm::mat4 &_view, glm::mat4 &_projection);


int main()
{
	//----------------------------------------------------------------Window initial
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

	//----------------------------------------------------------------register
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	//----------------------------------------------------------------object buffer
	// plane VAO
	glGenVertexArrays(1, &planeVAO);
	glBindVertexArray(planeVAO);
	unsigned int planeVBO;
	glGenBuffers(1,&planeVBO);
	glBindBuffer(GL_ARRAY_BUFFER,planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14* sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
	glBindVertexArray(0);

	//----------------------------------------------------------------framebuffers
	unsigned int directLightDepthMapFBO;
	glGenFramebuffers(1, &directLightDepthMapFBO);
	unsigned int directLightDepthMap;
	// - create depth map texture for direct Light
	glGenTextures(1, &directLightDepthMap);
	glBindTexture(GL_TEXTURE_2D, directLightDepthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glBindFramebuffer(GL_FRAMEBUFFER, directLightDepthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, directLightDepthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	unsigned int pointLightDepthMapFBO;
	glGenFramebuffers(1, &pointLightDepthMapFBO);
	// - create depth cubemap texture for point light
	unsigned int pointLightDepthCubemap;
	glGenTextures(1, &pointLightDepthCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, pointLightDepthCubemap);
	for (unsigned int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// - attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, pointLightDepthMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, pointLightDepthCubemap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	//----------------------------------------------------------------texture
	planeDiffuse=loadTexture("texture/floor_diff.jpg");
	planeNormal = loadTexture("texture/floor_norm.jpg");
	planeSpecular = loadTexture("texture/floor_spec.jpg");
	boxDiffuse = loadTexture("texture/box_diff.jpg");
	boxNormal = loadTexture("texture/box_norm.jpg");
	boxSpecular = loadTexture("texture/box_spec.jpg");
	
	//----------------------------------------------------------------Light
	float near_plane = 0.5f, far_plane = 25.0f;
	// - direct light
	glm::vec3 directlightPos(-2.0f, 4.0f, -1.0f);
	glm::mat4 directlightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	glm::mat4 directlightView = glm::lookAt(directlightPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	// - point light
	glm::vec3 pointlightPos(0.0f, 4.0f, -2.0f);
	glm::mat4 pointlightPorjection = glm::perspective(glm::radians(90.0f), float(SHADOW_WIDTH) / float(SHADOW_HEIGHT), near_plane, far_plane);
	std::vector<glm::mat4> pointlightTransforms;
	pointlightTransforms.push_back(pointlightPorjection // +x
		* glm::lookAt(pointlightPos, pointlightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
	pointlightTransforms.push_back(pointlightPorjection // -x
		* glm::lookAt(pointlightPos, pointlightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
	pointlightTransforms.push_back(pointlightPorjection // +y
		* glm::lookAt(pointlightPos, pointlightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
	pointlightTransforms.push_back(pointlightPorjection // -y
		* glm::lookAt(pointlightPos, pointlightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
	pointlightTransforms.push_back(pointlightPorjection // +z
		* glm::lookAt(pointlightPos, pointlightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
	pointlightTransforms.push_back(pointlightPorjection // -z
		* glm::lookAt(pointlightPos, pointlightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f)));

	//----------------------------------------------------------------shader
	// - lighting shader
	Shader ourShader("shader/shader.vs", "shader/shader.fs");
	ourShader.use();
	ourShader.setInt("material.diffuse",0);//bind with the texture
	ourShader.setInt("material.specular", 1);
	ourShader.setInt("material.normal", 4);
	ourShader.setFloat("material.shininess", 32.0f);
	ourShader.setVec3("directLight.ambient", 0.03f, 0.03f, 0.03f);
	ourShader.setVec3("directLight.diffuse", 0.5f, 0.5f, 0.5f);
	ourShader.setVec3("directLight.specular", 0.5f, 0.5f, 0.5f);
	ourShader.setInt("depthMap", 2);
	ourShader.setVec3("pointLight.ambient", 0.03f, 0.03f, 0.03f);
	ourShader.setVec3("pointLight.diffuse", 0.5f, 0.5f, 0.5f);
	ourShader.setVec3("pointLight.specular", 0.5f, 0.5f, 0.5f);
	ourShader.setInt("depthCubeMap", 3);
	ourShader.setFloat("far_plane", far_plane);
	// - pointlight depth buffer shader
	Shader pointlightShader("shader/pointlightshader.vs", "shader/pointlightshader.fs","shader/pointlightshader.gs");
	pointlightShader.use();
	// - directlight depth buffer shader
	Shader directlightshader("shader/directlightshader.vs", "shader/directlightshader.fs");
	directlightshader.use();
	// - debug shader
	//Shader depthShader("shader/debug_depth_quad.vs", "shader/debug_depth_quad.fs");
	//depthShader.use();
	//depthShader.setInt("depthMap", 0);

	//----------------------------------------------------------------ogl global
	glEnable(GL_DEPTH_TEST);

	//----------------------------------------------------------------render loop
	while (!glfwWindowShouldClose(window))
	{
		// - frametime
		currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// - input
		processInput(window);

		// - reset view port
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// - camera parameter
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)screenWidth / screenHeight, 0.1f, 1000.0f);

		// - render to direct light depth map
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, directLightDepthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		directlightshader.use();
		RenderScene(directlightshader, directlightView, directlightProjection);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// - render to point light depth cube map
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, pointLightDepthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		pointlightShader.use();
		for (unsigned int i = 0; i < 6; ++i)
			pointlightShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", pointlightTransforms[i]);
		pointlightShader.setVec3("lightPos", pointlightPos);
		pointlightShader.setFloat("far_plane", far_plane);
		RenderScene(pointlightShader, view, projection);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		// - debug depth
		//glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//depthShader.use();
		//depthShader.setFloat("near_plane", near_plane);
		//depthShader.setFloat("far_plane", far_plane);
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, depthMap);
		//renderQuad();

		// - then render scene as normal with shadow mapping (using depth map)
		glViewport(0, 0, screenWidth, screenHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ourShader.use();
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, directLightDepthMap);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_CUBE_MAP, pointLightDepthCubemap);
		ourShader.setMat4("lightSpaceMatrix", directlightProjection * directlightView);
		ourShader.setVec3("viewPos", camera.Position);
		ourShader.setVec3("directLight.direction", directlightPos-glm::vec3(0.0f));
		ourShader.setVec3("pointLight.position", pointlightPos);
		RenderScene(ourShader, view, projection);

		
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

std::vector<glm::vec3> calculateTBN(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 pos3, glm::vec2 uv1, glm::vec2 uv2, glm::vec2 uv3)
{
	glm::vec3 tangent1, bitangent1;
	// tiangle
	// edge and delta UV pairs 1/2
	glm::vec3 edge1 = pos2 - pos1;
	glm::vec3 edge2 = pos3 - pos1;
	glm::vec2 deltaUV1 = uv2 - uv1;
	glm::vec2 deltaUV2 = uv3 - uv1;

	GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
	tangent1 = glm::normalize(tangent1);

	bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
	bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
	bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
	bitangent1 = glm::normalize(bitangent1);
	std::vector<glm::vec3> result;
	result.push_back(tangent1); result.push_back(bitangent1);
	return result;
}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		glm::vec3 pos1(-1.0f, 1.0f, 0.0f);
		glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
		glm::vec3 pos3(1.0f, -1.0f, 0.0f);
		glm::vec3 pos4(1.0f, 1.0f, 0.0f);
		// texture coordinates
		glm::vec2 uv1(0.0f, 1.0f);
		glm::vec2 uv2(0.0f, 0.0f);
		glm::vec2 uv3(1.0f, 0.0f);
		glm::vec2 uv4(1.0f, 1.0f);
		// normal vector
		glm::vec3 nm(0.0f, 0.0f, 1.0f);

		// calculate tangent/bitangent vectors of both triangles
		glm::vec3 tangent1, bitangent1;
		glm::vec3 tangent2, bitangent2;
		// triangle 1
		// ----------
		glm::vec3 edge1 = pos2 - pos1;
		glm::vec3 edge2 = pos3 - pos1;
		glm::vec2 deltaUV1 = uv2 - uv1;
		glm::vec2 deltaUV2 = uv3 - uv1;

		GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangent1 = glm::normalize(tangent1);

		bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		bitangent1 = glm::normalize(bitangent1);

		// triangle 2
		// ----------
		edge1 = pos3 - pos1;
		edge2 = pos4 - pos1;
		deltaUV1 = uv3 - uv1;
		deltaUV2 = uv4 - uv1;

		f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangent2 = glm::normalize(tangent2);


		bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		bitangent2 = glm::normalize(bitangent2);


		float quadVertices[] = {
			// positions            // normal         // texcoords  // tangent                          // bitangent
			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
			pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
			pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
			pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
			pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
		};
		// -setup quad VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
	if (cubeVAO == 0)
	{
		float verticesBase[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			// bottom face
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			// top face
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};
		// - calcute each TB vector for the each primitive
		std::vector<glm::vec3> TBs;
		for (int i = 0; i < 12; i++)
		{
			std::vector<glm::vec3> tempt = calculateTBN(
				glm::vec3(verticesBase[24 * i + 8 * 0 + 0], verticesBase[24 * i + 8 * 0 + 1], verticesBase[24 * i + 8 * 0 + 2])
				, glm::vec3(verticesBase[24 * i + 8 * 1 + 0], verticesBase[24 * i + 8 * 1 + 1], verticesBase[24 * i + 8 * 1 + 2])
				, glm::vec3(verticesBase[24 * i + 8 * 2 + 0], verticesBase[24 * i + 8 * 2 + 1], verticesBase[24 * i + 8 * 2 + 2])
				, glm::vec2(verticesBase[24 * i + 8 * 0 + 6], verticesBase[24 * i + 8 * 0 + 7])
				, glm::vec2(verticesBase[24 * i + 8 * 1 + 6], verticesBase[24 * i + 8 * 1 + 7])
				, glm::vec2(verticesBase[24 * i + 8 * 2 + 6], verticesBase[24 * i + 8 * 2 + 7]));
			TBs.push_back(tempt[0]);
			TBs.push_back(tempt[1]);
		}
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, TBs[0].x,TBs[0].y,TBs[0].z,TBs[1].x,TBs[1].y,TBs[1].z,// bottom-left
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, TBs[0].x,TBs[0].y,TBs[0].z,TBs[1].x,TBs[1].y,TBs[1].z,// top-right
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, TBs[0].x,TBs[0].y,TBs[0].z,TBs[1].x,TBs[1].y,TBs[1].z,// bottom-right         
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, TBs[2].x,TBs[2].y,TBs[2].z,TBs[3].x,TBs[3].y,TBs[3].z,// top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, TBs[2].x,TBs[2].y,TBs[2].z,TBs[3].x,TBs[3].y,TBs[3].z,// bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, TBs[2].x,TBs[2].y,TBs[2].z,TBs[3].x,TBs[3].y,TBs[3].z,// top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, TBs[4].x,TBs[4].y,TBs[4].z,TBs[5].x,TBs[5].y,TBs[5].z,// bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, TBs[4].x,TBs[4].y,TBs[4].z,TBs[5].x,TBs[5].y,TBs[5].z,// bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, TBs[4].x,TBs[4].y,TBs[4].z,TBs[5].x,TBs[5].y,TBs[5].z,// top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, TBs[6].x,TBs[6].y,TBs[6].z,TBs[7].x,TBs[7].y,TBs[7].z,// top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, TBs[6].x,TBs[6].y,TBs[6].z,TBs[7].x,TBs[7].y,TBs[7].z,// top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, TBs[6].x,TBs[6].y,TBs[6].z,TBs[7].x,TBs[7].y,TBs[7].z,// bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, TBs[8].x,TBs[8].y,TBs[8].z,TBs[9].x,TBs[9].y,TBs[9].z,// top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, TBs[8].x,TBs[8].y,TBs[8].z,TBs[9].x,TBs[9].y,TBs[9].z,// top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, TBs[8].x,TBs[8].y,TBs[8].z,TBs[9].x,TBs[9].y,TBs[9].z,// bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, TBs[10].x,TBs[10].y,TBs[10].z,TBs[11].x,TBs[11].y,TBs[11].z,// bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, TBs[10].x,TBs[10].y,TBs[10].z,TBs[11].x,TBs[11].y,TBs[11].z,// bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, TBs[10].x,TBs[10].y,TBs[10].z,TBs[11].x,TBs[11].y,TBs[11].z,// top-right
			// right face
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, TBs[12].x,TBs[12].y,TBs[12].z,TBs[13].x,TBs[13].y,TBs[13].z,// top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, TBs[12].x,TBs[12].y,TBs[12].z,TBs[13].x,TBs[13].y,TBs[13].z,// bottom-right
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, TBs[12].x,TBs[12].y,TBs[12].z,TBs[13].x,TBs[13].y,TBs[13].z,// top-right         
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, TBs[14].x,TBs[14].y,TBs[14].z,TBs[15].x,TBs[15].y,TBs[15].z,// bottom-right
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, TBs[14].x,TBs[14].y,TBs[14].z,TBs[15].x,TBs[15].y,TBs[15].z,// top-left
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, TBs[14].x,TBs[14].y,TBs[14].z,TBs[15].x,TBs[15].y,TBs[15].z,// bottom-left     
			// bottom face
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, TBs[16].x,TBs[16].y,TBs[16].z,TBs[17].x,TBs[17].y,TBs[17].z,// top-right
			 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, TBs[16].x,TBs[16].y,TBs[16].z,TBs[17].x,TBs[17].y,TBs[17].z,// top-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, TBs[16].x,TBs[16].y,TBs[16].z,TBs[17].x,TBs[17].y,TBs[17].z,// bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, TBs[18].x,TBs[18].y,TBs[18].z,TBs[19].x,TBs[19].y,TBs[19].z,// bottom-left
			-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, TBs[18].x,TBs[18].y,TBs[18].z,TBs[19].x,TBs[19].y,TBs[19].z,// bottom-right
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, TBs[18].x,TBs[18].y,TBs[18].z,TBs[19].x,TBs[19].y,TBs[19].z,// top-right
			// top face
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, TBs[20].x,TBs[20].y,TBs[20].z,TBs[21].x,TBs[21].y,TBs[21].z,// top-left
			 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, TBs[20].x,TBs[20].y,TBs[20].z,TBs[21].x,TBs[21].y,TBs[21].z,// bottom-right
			 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, TBs[20].x,TBs[20].y,TBs[20].z,TBs[21].x,TBs[21].y,TBs[21].z,// top-right     
			 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, TBs[22].x,TBs[22].y,TBs[22].z,TBs[23].x,TBs[23].y,TBs[23].z,// bottom-right
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, TBs[22].x,TBs[22].y,TBs[22].z,TBs[23].x,TBs[23].y,TBs[23].z,// top-left
			-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f, TBs[22].x,TBs[22].y,TBs[22].z,TBs[23].x,TBs[23].y,TBs[23].z,// bottom-left        
		};
		// -setup cube VAO
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		glBindVertexArray(cubeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void RenderScene(Shader& shader, glm::mat4& _view, glm::mat4& _projection)
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;

	// -setup current rendering view and projection matrix
	view = _view;
	projection = _projection;

	//cube mat
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, boxDiffuse);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, boxSpecular);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, boxNormal);

	// -render cube 1
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
	model = glm::rotate(model, glm::radians(100.0f*(float)glfwGetTime()), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMat4("model", model);
	shader.setMat4("view", view);
	shader.setMat4("projection", projection);
	renderCube();

	// -render cube 2
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(2.0f, 0.5f, 1.0f));
	model = glm::rotate(model, glm::radians(30.0f*(float)glfwGetTime() + 50.0f), glm::normalize(glm::vec3(0.0, 1.0, 0.0)));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMat4("model", model);
	shader.setMat4("view", view);
	shader.setMat4("projection", projection);
	renderCube();

	// -render cube 3
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.25));
	shader.setMat4("model", model);
	shader.setMat4("view", view);
	shader.setMat4("projection", projection);
	renderCube();

	// - ground mat
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, planeDiffuse);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, planeSpecular);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, planeNormal);
	// -render ground
	model = glm::mat4(1.0f);
	shader.setMat4("model", model);
	shader.setMat4("view", view);
	shader.setMat4("projection", projection);
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}