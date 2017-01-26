#include "CustomPlane.h"

CustomPlane::CustomPlane(GLfloat skybox_scale)
{
	// Set up our vertex data (and buffer(s)) and attribute pointers
	GLfloat vertices[] = {
		// First triangle
		0.5f * skybox_scale,  0.5f * skybox_scale, 0.0f,  // Top Right
		0.5f * skybox_scale, -0.5f * skybox_scale, 0.0f,  // Bottom Right
		-0.5f * skybox_scale,  0.5f * skybox_scale, 0.0f,  // Top Left

		// Second triangle
		0.5f * skybox_scale, -0.5f * skybox_scale, 0.0f,  // Bottom Right
		-0.5f * skybox_scale, -0.5f * skybox_scale, 0.0f,  // Bottom Left
		-0.5f * skybox_scale,  0.5f * skybox_scale, 0.0f   // Top Left
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	// Bind our Vertex Array Object first, then bind and set our buffers and pointers.
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0); // Unbind VAO
}


CustomPlane::~CustomPlane()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

void CustomPlane::render()
{
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	// (mode, vertex count, type, element array buffer offset)
	glBindVertexArray(0);
}
