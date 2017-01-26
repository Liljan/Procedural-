// Std. Includes
#include <string>
#include <vector>

// GLEW
//#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include "CustomShader.h"
#include "CustomCamera.h"
#include "CustomPlane.h"

#include "Sphere.h"

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Windows shite
#include <Windows.h>

// Properties
GLuint WIDTH = 1920, HEIGHT = 1080;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scan_code, int action, int mode);
void scroll_callback(GLFWwindow* window, double x_offset, double y_offset);
void mouse_callback(GLFWwindow* window, double x_pos, double y_pos);
void camera_movement();
void drawGUI();

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool keys[1024];
GLfloat last_x = WIDTH / 2.0f, last_y = HEIGHT / 2.f;
bool first_mouse = true;

// Objects
std::vector<CustomPlane*> skybox;
float scale = 10.0f;

GLfloat delta_time = 0.0f;
GLfloat last_time = 0.0f;

static const float M_PI = 3.141592653f;
const float M_PI_HALF = M_PI/2.0f;
static const float DEGREE_TO_RADIAN = M_PI / 180.0f;
static const float RADIAN_TO_DEGREE = 180.0f / M_PI;
static const char file_name_buffer[30] = {};

static char load_buffer[256] = "";
static char save_buffer[256] = "";
static char files_buffer[1024] = "";

static const std::string FILE_ENDING = ".ass";

// ________ TERRAIN _________
// Procedural related variables
int segments = 32;
float elevation = 0.1f;
float radius = 0.01f;
float lacunarity = 1.0f;
float vert_frequency = 4.0f;
float frag_frequency = 4.0f;
int octaves = 6;
int seed = 0;

float color_water_1[3] = { 1.0f,1.0f,1.0f };
float color_water_2[3] = { 1.0f,1.0f,1.0f };
float color_ground_1[3] = { 1.0f,1.0f,1.0f };
float color_ground_2[3] = { 1.0f,1.0f,1.0f };
float color_mountain_1[3] = { 1.0f,1.0f,1.0f };
float color_mountain_2[3] = { 0.05f,0.01f,0.03f };

bool use_perlin = true;
bool use_simplex = false;
bool use_cell = false;

// ________ SKY _________
// Procedural related variables
bool enable_sky = true;

float sky_frequency = 4.0f;
int sky_seed = 0;
int sky_octaves = 6;

Sphere* sphere;
Sphere* sky_sphere;

void list_files()
{
	std::vector<std::string> return_file_name;
	WIN32_FIND_DATA file_info;
	HANDLE h_find;

	std::string fullPath = "*" + FILE_ENDING;
	h_find = FindFirstFile(fullPath.c_str(), &file_info);

	file_info;

	if (h_find != INVALID_HANDLE_VALUE) {
		return_file_name.push_back(file_info.cFileName);
		while (FindNextFile(h_find, &file_info) != 0) {
			return_file_name.push_back(file_info.cFileName);
		}
	}

	std::stringstream ss;
	for (int i = 0; i < return_file_name.size(); ++i)
		ss << return_file_name[i] << std::endl;

	memset(files_buffer, 0, strlen(files_buffer));
	strcpy(files_buffer, ss.str().c_str());
}


void color_to_file(std::ofstream &file, float color[3]) {
	file << color[0] << std::endl;
	file << color[1] << std::endl;
	file << color[2] << std::endl;
}


void file_to_color(std::ifstream &file, float color[3]) {
	file >> color[0];
	file >> color[1];
	file >> color[2];
}


void load_file(std::string file_name)
{
	try
	{
		std::ifstream infile(file_name + FILE_ENDING, std::ifstream::binary);

		infile >> segments;
		infile >> elevation;
		infile >> radius;
		infile >> lacunarity;
		infile >> vert_frequency;
		infile >> octaves;
		infile >> seed;

		// colors
		file_to_color(infile, color_water_1);
		file_to_color(infile, color_water_2);
		file_to_color(infile, color_ground_1);
		file_to_color(infile, color_ground_2);
		file_to_color(infile, color_mountain_1);
		file_to_color(infile, color_mountain_2);

		infile >> use_perlin;
		infile >> use_simplex;
		infile >> use_cell;

		infile.close();
		delete sphere;
		sphere = new Sphere(0.0f, 0.0f, 0.0f, 1.0f, segments);
	}
	catch (const std::exception&)
	{
		std::cout << "Error loading" << std::endl;
	}
}


void save_file(std::string file_name) {

	try
	{
		std::ofstream outfile(file_name + FILE_ENDING, std::ofstream::binary);
		outfile << segments << std::endl;
		outfile << elevation << std::endl;
		outfile << radius << std::endl;
		outfile << lacunarity << std::endl;
		outfile << vert_frequency << std::endl;
		outfile << octaves << std::endl;
		outfile << seed << std::endl;

		// colors
		color_to_file(outfile, color_water_1);
		color_to_file(outfile, color_water_2);
		color_to_file(outfile, color_ground_1);
		color_to_file(outfile, color_ground_2);
		color_to_file(outfile, color_mountain_1);
		color_to_file(outfile, color_mountain_2);

		outfile << use_perlin << std::endl;
		outfile << use_simplex << std::endl;
		outfile << use_cell << std::endl;

		outfile.close();
	}
	catch (const std::exception&)
	{
		std::cout << "Error saving" << std::endl;
	}
}

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
	Shader terrain_shader("shaders/terrain_vert.glsl", "shaders/terrain_frag.glsl");
	//Shader sky_shader("shaders/sky_vert.glsl", "shaders/sky_frag.glsl");

	// ___________ VARIABLES _____________________

	// Light related variables
	float light_position[3] = { 1.0f, 1.0f, 1.0f };
	float light_intensity = 1.0f;
	float shininess = 1.0f;

	// GUI related variables
	bool show_tooltips = true;
	bool draw_wireframe = false;

	// Time related variables
	float time;
	float sky_speed = 1.0f;
	bool is_paused = false;

	// other shite
	float rotation_degrees[2] = { 0.0f,0.0f };
	float rotation_radians[2] = { 0.0f,0.0f };

	// Initialize geometry objects
	sphere = new Sphere(0.0f, 0.0f, 0.0f, 1.0f, segments);
	sky_sphere = new Sphere(0.0f, 0.0f, 0.0f, 1.0f, 32);

	// Init background
	skybox.push_back(new CustomPlane(scale, glm::vec3(0, 0, scale / 2.f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f))); // front
	skybox.push_back(new CustomPlane(scale, glm::vec3(0, 0, -scale / 2.f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f))); //back
	skybox.push_back(new CustomPlane(scale, glm::vec3(0, scale / 2.f, 0.0f), M_PI_HALF, glm::vec3(1.0f, 0.0f, 0.0f))); //top
	skybox.push_back(new CustomPlane(scale, glm::vec3(0, -scale / 2.f, 0.0f), M_PI_HALF, glm::vec3(1.0f, 0.0f, 0.0f))); //bottom
	skybox.push_back(new CustomPlane(scale, glm::vec3(scale / 2.f, 0.0f, 0.0f), M_PI_HALF, glm::vec3(0.0f, 1.0f, 0.0f))); // right
	skybox.push_back(new CustomPlane(scale, glm::vec3(-scale / 2.f, 0.0f, 0.0f), M_PI_HALF, glm::vec3(0.0f, 1.0f, 0.0f))); // left

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
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create camera and projection
		glm::mat4 view, projection, model;
		view = camera.GetViewMatrix();
		projection = glm::perspective(camera.zoom, (float)WIDTH / (float)HEIGHT, 0.1f, 1000.0f);

		if (draw_wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		GLint Loc_model, Loc_view, Loc_projection;

		// ______________ Planet Geometry ________________
		terrain_shader.Use();

		Loc_model = glGetUniformLocation(terrain_shader.Program, "model");
		Loc_view = glGetUniformLocation(terrain_shader.Program, "view");
		Loc_projection = glGetUniformLocation(terrain_shader.Program, "projection");

		glUniformMatrix4fv(Loc_view, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(Loc_projection, 1, GL_FALSE, glm::value_ptr(projection));
		
		model = glm::mat4x4();
		model = glm::translate(model, sphere->m_position);
		glUniformMatrix4fv(Loc_model, 1, GL_FALSE, glm::value_ptr(model));

		sphere->render();

		// ______________ Background plane ________________

		// Draw our first plane
		star_shader.Use();

		Loc_model = glGetUniformLocation(star_shader.Program, "model");
		Loc_view = glGetUniformLocation(star_shader.Program, "view");
		Loc_projection = glGetUniformLocation(star_shader.Program, "projection");
		// Pass the matrices to the shader
		glUniformMatrix4fv(Loc_view, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(Loc_projection, 1, GL_FALSE, glm::value_ptr(projection));

		for (int i = 0; i < skybox.size(); ++i) {
			model = glm::mat4x4();
			model = glm::translate(model, skybox[i]->m_position);
			model = glm::rotate(model, skybox[i]->m_angle, skybox[i]->m_rotation);
			glUniformMatrix4fv(Loc_model, 1, GL_FALSE, glm::value_ptr(model));
			skybox[i]->render();
		}
		

		// Swap the buffers
		glfwSwapBuffers(window);
	}
	// Properly de-allocate all resources once they've outlived their purpose
	//delete background_plane;
	for (int i = 0; i < skybox.size(); ++i)
		delete skybox[i];

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