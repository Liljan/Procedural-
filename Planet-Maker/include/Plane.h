#pragma once
#include "Gl/glew.h"
#include "glm/glm.hpp"

class Plane
{
public:
	Plane()
	{
		vao = 0;
		vertexbuffer = 0;
		indexbuffer = 0;
		vertexarray = NULL;
		indexarray = NULL;
		nverts = 0;
		ntris = 0;
	};

	Plane(float x, float y, float z, float width, float height);
	~Plane(void);

	glm::vec3 getNormal() { return normal; }
	void setNormal(glm::vec3 n) { normal = n; }
	void setOrientation(glm::vec3 o) { orientation = o; }
	glm::vec3* getPosition() { return &position; }
	void setAngularPosition(float a) { angularPosition = a; }

	void render();
	glm::vec2 getDim() { return dim; }
	glm::vec3 getOrientation() { return orientation; }
	float getAngularPosition() { return angularPosition; }

private:
	GLuint vao;          // Vertex array object, the main handle for geometry
	int nverts; // Number of vertices in the vertex array
	int ntris;  // Number of triangles in the index array (may be zero)
	GLuint vertexbuffer; // Buffer ID to bind to GL_ARRAY_BUFFER
	GLuint indexbuffer;  // Buffer ID to bind to GL_ELEMENT_ARRAY_BUFFER
	GLfloat *vertexarray; // Vertex array on interleaved format: x y z nx ny nz s t
	GLuint *indexarray;   // Element index array

	glm::vec2 dim;
	glm::vec3 normal;

	glm::vec3 position;

	glm::vec3 orientation;
	glm::vec3 rotAxis;
	float angularPosition;
};