#pragma once

// STD Includes
#include <vector>
#include <cmath>

// GL Includes
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// Default camera values
const GLfloat YAW = -90.0f;
const GLfloat PITCH = 0.0f;
const GLfloat SPEED = 3.0f;
const GLfloat SENSITIVTY = 0.25f;
const GLfloat ZOOM = 45.0f;


// An abstract camera class that processes input and calculates the corresponding Eular Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
	// Camera Attributes
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 world_up;
	// Eular Angles
	GLfloat yaw;
	GLfloat pitch;
	// Camera options
	GLfloat movement_speed;
	GLfloat mouse_sensitivity;
	GLfloat zoom;

	// Constructor with vectors
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), GLfloat yaw = YAW, GLfloat pitch = PITCH) : front(glm::vec3(0.0f, 0.0f, -1.0f)), movement_speed(SPEED), mouse_sensitivity(SENSITIVTY), zoom(ZOOM)
	{
		this->position = position;
		this->world_up = up;
		this->yaw = yaw;
		this->pitch = pitch;
		this->update_camera_vectors();
	}
	// Constructor with scalar values
	Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw, GLfloat pitch) : front(glm::vec3(0.0f, 0.0f, -1.0f)), movement_speed(SPEED), mouse_sensitivity(SENSITIVTY), zoom(ZOOM)
	{
		this->position = glm::vec3(posX, posY, posZ);
		this->world_up = glm::vec3(upX, upY, upZ);
		this->yaw = yaw;
		this->pitch = pitch;
		this->update_camera_vectors();
	}

	// Returns the view matrix calculated using Eular Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(this->position, this->position + this->front, this->up);
	}

	// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void process_keyboard(Camera_Movement direction, GLfloat deltaTime)
	{
		GLfloat velocity = this->movement_speed * deltaTime;
		if (direction == FORWARD)
			this->position += this->front * velocity;
		if (direction == BACKWARD)
			this->position -= this->front * velocity;
		if (direction == LEFT)
			this->position -= this->right * velocity;
		if (direction == RIGHT)
			this->position += this->right * velocity;
	}

	// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void process_mouse_movement(GLfloat x_offset, GLfloat y_offset, GLboolean constrain_pitch = true)
	{
		x_offset *= this->mouse_sensitivity;
		y_offset *= this->mouse_sensitivity;

		this->yaw += x_offset;
		this->pitch += y_offset;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrain_pitch)
		{
			if (this->pitch > 89.0f)
				this->pitch = 89.0f;
			if (this->pitch < -89.0f)
				this->pitch = -89.0f;

		}

		// Update Front, Right and Up Vectors using the updated Eular angles
		this->update_camera_vectors();
	}

	// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void process_mouse_scroll(GLfloat y_offset, float dt)
	{
		if (this->zoom >= 1.0f && this->zoom <= 45.0f)
			this->zoom -= y_offset * dt;

		// clamp

		zoom = fmax(zoom, 1.0f);
		zoom = fmin(zoom, 45.0f);

		/*if (this->zoom <= 1.0f)
			this->zoom = 1.0f;
		if (this->zoom >= 45.0f)
			this->zoom = 45.0f;*/
	}

private:
	// Calculates the front vector from the Camera's (updated) Eular Angles
	void update_camera_vectors()
	{
		// Calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
		front.y = sin(glm::radians(this->pitch));
		front.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
		this->front = glm::normalize(front);
		// Also re-calculate the Right and Up vector
		this->right = glm::normalize(glm::cross(this->front, this->world_up));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		this->up = glm::normalize(glm::cross(this->right, this->front));
	}
};