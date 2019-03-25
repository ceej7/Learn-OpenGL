#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Shader.h"
//const char *vertexShaderSource="#version 330 core\n\
//layout(location = 0) in vec3 aPos;\n\
//void main()\n\
//{\n\
//	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n\
//}";
//
//const char *fragmentShaderSource = "#version 330 core\n\
//out vec4 FragColor;\n\
//uniform vec4 ourColor;\n\
//void main()\n\
//{\n\
//	FragColor = ourColor;\n\
//}\n\
//";
const char *vertexShaderSource = "#version 330 core\n\
layout(location = 0) in vec3 aPos;\n\
layout (location = 1) in vec3 aColor;\n\
out vec3 ourColor;\n\
void main()\n\
{\n\
	gl_Position = vec4(aPos, 1.0);\n\
	ourColor = aColor;\n\
}";

const char *fragmentShaderSource = "#version 330 core\n\
out vec4 FragColor;\n\
in vec3 ourColor;\n\
void main()\n\
{\n\
	FragColor = vec4(ourColor,1.0);\n\
}\n\
";


//vertex
//float vertices[] = {
//	// first triangle
//		 0.5f,  0.5f, 0.0f,  // top right
//		 0.5f, -0.5f, 0.0f,  // bottom right
//		-0.5f,  0.5f, 0.0f,  // top left 
//		// second triangle
//		 0.5f, -0.5f, 0.0f,  // bottom right
//		-0.5f, -0.5f, 0.0f,  // bottom left
//		-0.5f,  0.5f, 0.0f   // top left
//};
//unsigned int indices[] = {  // note that we start from 0!
//	0, 1, 2,   // first triangle
//	0, 2, 4    // second triangle
//};
float vertices[] = {
	// positions         // colors
	 0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // bottom right
	-0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // bottom left
	 0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // top 
};


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

int main()
{
	//////////////////////////Window initial
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
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
	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);



	/////////////////////////Buffer issue
	//generate a VAO
	//unsigned int VAO;
	//glGenVertexArrays(1, &VAO);
	////generate a VBO
	//unsigned int VBO;
	//glGenBuffers(1, &VBO);
	////generate a EBO
	//unsigned int EBO;
	//glGenBuffers(1, &EBO);
	//// bind Vertex Array Object
	//glBindVertexArray(VAO);
	////buffer for VBO
	//glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	////buffer for EBO
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	////set our vertex attributes pointers
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0));
	//glEnableVertexAttribArray(0);
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	//generate a VBO
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	// bind Vertex Array Object
	glBindVertexArray(VAO);
	//buffer for VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	//set our vertex attributes pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(0));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);


	////////////////////////Shader Issue
	////compile vertex shader
	//unsigned int vertexShader;
	//vertexShader = glCreateShader(GL_VERTEX_SHADER);
	////attach the shader to vs object and compile it
	//glShaderSource(vertexShader, 1,&vertexShaderSource, NULL);
	//glCompileShader(vertexShader);
	////check compile error
	//int  success;
	//char infoLog[512];
	//glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	//if (!success)
	//{
	//	glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
	//	std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	//}
	////compile fragment shader
	//unsigned int fragmentShader;
	//fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	//glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	//glCompileShader(fragmentShader);
	//glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	//if (!success)
	//{
	//	glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
	//	std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	//}
	////link and activate
	//unsigned int shaderProgram;
	//shaderProgram = glCreateProgram();
	//glAttachShader(shaderProgram, vertexShader);
	//glAttachShader(shaderProgram, fragmentShader);
	//glLinkProgram(shaderProgram);
	////glUseProgram(shaderProgram);//postpone to rendering
	//glGetShaderiv(shaderProgram, GL_LINK_STATUS, &success);
	//if (!success)
	//{
	//	glGetShaderInfoLog(shaderProgram, 512, NULL, infoLog);
	//	std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n" << infoLog << std::endl;
	//}
	////delete shader object
	//glDeleteShader(vertexShader);
	//glDeleteShader(fragmentShader);


	Shader ourShader("shader.vs", "shader.fs");


	while (!glfwWindowShouldClose(window))
	{
		// input
		processInput(window);

		// rendering commands here
		glClearColor(0.4f, 0.2f, 0.8f, 1.0f);//state-setting
		glClear(GL_COLOR_BUFFER_BIT);//state-using


		//float timeValue = glfwGetTime();
		//float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
		//int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
		//glUseProgram(shaderProgram);
		//glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);
		//glBindVertexArray(VAO);
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		//glBindVertexArray(0);

		//glUseProgram(shaderProgram);
		//glBindVertexArray(VAO);
		//glDrawArrays(GL_TRIANGLES, 0, 3);
		//glBindVertexArray(0);

		ourShader.use();
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);

		// check and call events and swap the buffers
		glfwPollEvents();
		glfwSwapBuffers(window);
	}
	glfwTerminate();
	return 0;

}