#pragma once
#include <GL/glew.h>

#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Plane
{
public:
	Plane(GLfloat skybox_scale, glm::vec3 position, float angle, glm::vec3 rotation);
	~Plane();

	void render();

	GLfloat vertices[3*6];
	GLfloat skybox_scale = 1.0f;
	GLuint VBO, VAO;

	glm::vec3 m_position;
	float m_angle;
	glm::vec3 m_rotation;

	GLfloat scale;

private:
	glm::mat4x4 model;
};

