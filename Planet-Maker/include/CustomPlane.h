#pragma once
#include <GL/glew.h>
#include <CustomShader.h>

#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class CustomPlane
{
public:
	CustomPlane(GLfloat skybox_scale);
	~CustomPlane();

	void render();

	GLfloat vertices[3*6];
	GLfloat skybox_scale = 1.0f;
	GLuint VBO, VAO;

	glm::vec3 m_position;
	glm::vec3 m_rotation;

	GLfloat scale;

private:
	glm::mat4x4 model;
};

