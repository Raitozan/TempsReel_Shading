#include "glad/include/glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>

#include <vector>
#include <iostream>
#include <random>
#include <sstream>
#include <fstream>
#include <string>
#include "stl.h"

static void error_callback(int /*error*/, const char* description)
{
	std::cerr << "Error: " << description << std::endl;
}

static void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

/* PARTICULES */
struct Particule {
	glm::vec3 position;
	glm::vec3 color;
	glm::vec3 speed;
};

std::vector<Particule> MakeParticules(const int n)
{
  std::default_random_engine generator;
  std::uniform_real_distribution<float> distribution01(0, 1);
  std::uniform_real_distribution<float> distributionWorld(-1, 1);

  std::vector<Particule> p;
  p.reserve(n);
  
  for(int i = 0; i < n; i++)
  {
	  p.push_back(Particule{
					 {
					  distributionWorld(generator),
					  distributionWorld(generator),
					  distributionWorld(generator)
					 },
					 {
					  distribution01(generator),
					  distribution01(generator),
					  distribution01(generator)
					 },
					 {0.f, 0.f, 0.f}
		  });
  }

  return p;
}

std::vector<glm::vec3> MakeNormals(std::vector<Triangle> triangles)
{
	std::vector<glm::vec3> normals;
	normals.reserve(triangles.size() * 3);
	for (auto triangle: triangles)
	{
		glm::vec3 normal = glm::cross((triangle.p1 - triangle.p0), (triangle.p2 - triangle.p0));
		normal = glm::normalize(normal);
		normals.push_back(normal);
		normals.push_back(normal);
		normals.push_back(normal);
	}
	return normals;
}

GLuint MakeShader(GLuint t, std::string path)
{
	std::cout << path << std::endl;
	std::ifstream file(path.c_str(), std::ios::in);
    std::ostringstream contents;
    contents << file.rdbuf();
    file.close();

	const auto content = contents.str();
	std::cout << content << std::endl;
	
	const auto s = glCreateShader(t);

	GLint sizes[] = {(GLint) content.size()};
	const auto data = content.data();

	glShaderSource(s, 1, &data, sizes);
	glCompileShader(s);

	GLint success;
	glGetShaderiv(s, GL_COMPILE_STATUS, &success);
	if(!success)
	{
		GLchar infoLog[512];
		GLsizei l;
		glGetShaderInfoLog(s, 512, &l, infoLog);

		std::cout << infoLog << std::endl;
	}

	return s;
}

GLuint AttachAndLink(std::vector<GLuint> shaders)
{
	const auto prg = glCreateProgram();
	for(const auto s : shaders)
	{
		glAttachShader(prg, s);
	}

	glLinkProgram(prg);

	GLint success;
	glGetProgramiv(prg, GL_LINK_STATUS, &success);
	if(!success)
	{
		GLchar infoLog[512];
		GLsizei l;
		glGetProgramInfoLog(prg, 512, &l, infoLog);

		std::cout << infoLog << std::endl;
	}

	return prg;
}

int main(void)
{
    GLFWwindow* window;
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

    window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    // NOTE: OpenGL error checks have been omitted for brevity

	if(!gladLoadGL()) {
		std::cerr << "Something went wrong!" << std::endl;
       exit(-1);
    }

	//Créations des points
	const size_t nParticules = 10000;
	auto particules = MakeParticules(nParticules);
	//Créations des triangles
	auto tris = ReadStl("logo.stl");
	auto nTriangles = tris.size();
	auto normals = MakeNormals(tris);
	//variables diverses
	float rotValue = 0.0f;

	//Shader Particules
	const auto vertexParticles = MakeShader(GL_VERTEX_SHADER, "shaderPart.vert");
	const auto fragmentParticles = MakeShader(GL_FRAGMENT_SHADER, "shaderPart.frag");

	const auto programParticles = AttachAndLink({vertexParticles, fragmentParticles});

	//====================

	//Shader Triangles
	const auto vertexTriangles = MakeShader(GL_VERTEX_SHADER, "shaderTris.vert");
	const auto fragmentTriangles = MakeShader(GL_FRAGMENT_SHADER, "shaderTris.frag");

	const auto programTriangles = AttachAndLink({ vertexTriangles, fragmentTriangles });

	//====================

	// Buffers Particules
	GLuint vboPart, vaoPart;
	glGenBuffers(1, &vboPart);
	glGenVertexArrays(1, &vaoPart);
	glBindVertexArray(vaoPart);
	glBindBuffer(GL_ARRAY_BUFFER, vboPart);
	glBufferData(GL_ARRAY_BUFFER, nParticules * sizeof(Particule), particules.data(), GL_STATIC_DRAW);

	//====================

	//Bindings Particules
	const auto indexPositionParticules = glGetAttribLocation(programParticles, "positionParticule");

	glVertexAttribPointer(indexPositionParticules, 3, GL_FLOAT, GL_FALSE, sizeof(Particule), nullptr);
	glEnableVertexAttribArray(indexPositionParticules);

	const auto indexColor = glGetAttribLocation(programParticles, "colorParticule");

	glVertexAttribPointer(indexColor, 3, GL_FLOAT, GL_FALSE, sizeof(Particule), (const void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(indexColor);

	//====================


	//Buffers Triangles
	GLuint vboTris, vaoTris;
	glGenBuffers(1, &vboTris);
	glGenVertexArrays(1, &vaoTris);
	glBindVertexArray(vaoTris);
	glBindBuffer(GL_ARRAY_BUFFER, vboTris);
	glBufferData(GL_ARRAY_BUFFER, 2 * nTriangles * sizeof(Triangle), nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, nTriangles * sizeof(Triangle), tris.data());
	glBufferSubData(GL_ARRAY_BUFFER, nTriangles * sizeof(Triangle), nTriangles * sizeof(Triangle), normals.data());

	//====================

	//Bindings Triangles
	const auto indexPositionVertTri = glGetAttribLocation(programTriangles, "positionVertTri");

	glVertexAttribPointer(indexPositionVertTri, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
	glEnableVertexAttribArray(indexPositionVertTri);

	const auto indexNormalVertTri = glGetAttribLocation(programTriangles, "normalVertTri");

	glVertexAttribPointer(indexNormalVertTri, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (const void*)(nTriangles * sizeof(Triangle)));
	glEnableVertexAttribArray(indexNormalVertTri);

	//====================

	glPointSize(0.5f);

	// camera and perspective
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	GLint uniView = glGetUniformLocation(programTriangles, "view");
	glProgramUniformMatrix4fv(programTriangles, uniView, 1, GL_FALSE, glm::value_ptr(view));

	glm::mat4 proj = glm::perspective(glm::radians(45.0f), 640.0f / 480.0f, 0.01f, 100.0f);
	GLint uniProj = glGetUniformLocation(programTriangles, "proj");
	glProgramUniformMatrix4fv(programTriangles, uniProj, 1, GL_FALSE, glm::value_ptr(proj));

	//==============================
	
    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT);

		//PARTICULES
		glUseProgram(programParticles);
		glBindVertexArray(vaoPart);

		std::default_random_engine generator;
		std::uniform_real_distribution<float> distributionWorld(-0.01f, 0.01f);
		for (int i = 0; i < nParticules; i++) {
			particules.at(i).position.x += distributionWorld(generator);
			particules.at(i).position.y += distributionWorld(generator);
			particules.at(i).position.z += distributionWorld(generator);
			if (particules.at(i).position.x >= 1 || particules.at(i).position.x <= -1)
				particules.at(i).position.x = -particules.at(i).position.x;
			if (particules.at(i).position.y >= 1 || particules.at(i).position.y <= -1)
				particules.at(i).position.y = -particules.at(i).position.y;
			if (particules.at(i).position.z >= 1 || particules.at(i).position.z <= -1)
				particules.at(i).position.z = -particules.at(i).position.z;
		}
		glBindBuffer(GL_ARRAY_BUFFER, vboPart);
		glBufferSubData(GL_ARRAY_BUFFER, 0, nParticules * sizeof(Particule), particules.data());

		glDrawArrays(GL_POINTS, 0, nParticules);

		//==========

		//TRIANGLES
		glUseProgram(programTriangles);
		glBindVertexArray(vaoTris);

		glm::mat4 transfoTris = glm::mat4(1.0f);
		rotValue += 1.0f;
		transfoTris = glm::rotate(transfoTris, glm::radians(rotValue), glm::vec3(0.0f, 0.5f, 1.0f));
		transfoTris = glm::scale(transfoTris, glm::vec3(0.01f, 0.01f, 0.01f));
		GLint uniTransfoTris = glGetUniformLocation(programTriangles, "transfo");
		glProgramUniformMatrix4fv(programTriangles, uniTransfoTris, 1, GL_FALSE, glm::value_ptr(transfoTris));

		glDrawArrays(GL_TRIANGLES, 0, nTriangles*3);

		//==========

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}