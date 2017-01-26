// Std. Includes
#include <string>

// GLEW
//#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include "CustomShader.h"
#include "CustomCamera.h"
#include "CustomPlane.h"

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Properties
GLuint WIDTH = 1920, HEIGHT = 1080;
GLfloat skybox_scale = 10.0f;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scan_code, int action, int mode);
void scroll_callback(GLFWwindow* window, double x_offset, double y_offset);
void mouse_callback(GLFWwindow* window, double x_pos, double y_pos);
void camera_movement();

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool keys[1024];
GLfloat last_x = WIDTH / 2.0f, last_y = HEIGHT / 2.f;
bool first_mouse = true;

// Objects
CustomPlane* background_plane;

GLfloat delta_time = 0.0f;
GLfloat last_time = 0.0f;

// The MAIN function, from here we start our application and run our Game loop
int main()
{
	// Init GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Procedural Planet Generator", nullptr, nullptr); // Windowed
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Initialize GLEW to setup the OpenGL Function pointers
	glewExperimental = GL_TRUE;
	glewInit();

	// Define the viewport dimensions
	glViewport(0, 0, WIDTH, HEIGHT);

	// Setup some OpenGL options
	glEnable(GL_DEPTH_TEST);

	// Setup and compile our shaders
	Shader star_shader("shaders/star_vert.glsl", "shaders/star_frag.glsl");

	// Initialize geometry objects
	background_plane = new CustomPlane(10.0f);

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Set frame time
		GLfloat currentFrame = glfwGetTime();
		delta_time = currentFrame - last_time;
		last_time = currentFrame;

		// Check and call events
		glfwPollEvents();
		camera_movement();

		// Clear the colorbuffer
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create camera and projection
		glm::mat4 view, projection, model;
		view = camera.GetViewMatrix();
		projection = glm::perspective(camera.zoom, (float)WIDTH / (float)HEIGHT, 0.1f, 1000.0f);


		// ______________ Background plane ________________

		// Draw our first plane
		star_shader.Use();

		GLint Loc_model, Loc_view, Loc_projection;

		Loc_model = glGetUniformLocation(star_shader.Program, "model");
		Loc_view = glGetUniformLocation(star_shader.Program, "view");
		Loc_projection = glGetUniformLocation(star_shader.Program, "projection");
		// Pass the matrices to the shader
		glUniformMatrix4fv(Loc_view, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(Loc_projection, 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::translate(model, glm::vec3(0,0,0));
		model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(Loc_model, 1, GL_FALSE, glm::value_ptr(model));
		// render plane HERE!

		background_plane->render();

		// Swap the buffers
		glfwSwapBuffers(window);
	}
	// Properly de-allocate all resources once they've outlived their purpose
	delete background_plane;


	glfwTerminate();
	return 0;
}

// Moves/alters the camera positions based on user input
void camera_movement()
{
	// Camera controls
	if (keys[GLFW_KEY_W])
		camera.process_keyboard(FORWARD, delta_time);
	if (keys[GLFW_KEY_S])
		camera.process_keyboard(BACKWARD, delta_time);
	if (keys[GLFW_KEY_A])
		camera.process_keyboard(LEFT, delta_time);
	if (keys[GLFW_KEY_D])
		camera.process_keyboard(RIGHT, delta_time);
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scan_code, int action, int mode)
{
	//cout << key << endl;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}

void mouse_callback(GLFWwindow* window, double x_pos, double y_pos)
{
	if (first_mouse)
	{
		last_x = x_pos;
		last_y = y_pos;
		first_mouse = false;
	}

	GLfloat xoffset = x_pos - last_x;
	GLfloat yoffset = last_y - y_pos;  // Reversed since y-coordinates go from bottom to left

	last_x = x_pos;
	last_y = y_pos;

	camera.process_mouse_movement(xoffset, yoffset);
}


void scroll_callback(GLFWwindow* window, double x_offset, double y_offset)
{
	camera.process_mouse_scroll(y_offset, delta_time);
}