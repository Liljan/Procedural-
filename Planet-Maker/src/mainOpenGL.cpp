// Std. Includes
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

// GLEW
//#define GLEW_STATIC
#include <GL/glew.h>

// Imgui
#include <imgui.h>
#include <imgui_impl_glfw.h>

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
GLuint WIDTH = 800, HEIGHT = 600;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scan_code, int action, int mode);
void scroll_callback(GLFWwindow* window, double x_offset, double y_offset);
void mouse_callback(GLFWwindow* window, double x_pos, double y_pos);
void camera_movement();
void draw_GUI();

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
const float M_PI_HALF = M_PI / 2.0f;
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

void GL_calls()
{
	glClearColor(0.01f, 0.01f, 0.01f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glDisable(GL_TEXTURE);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

	// Init ImGUI
	ImGui_ImplGlfw_Init(window, true);

	// Setup and compile our shaders
	Shader star_shader("shaders/star_vert.glsl", "shaders/star_frag.glsl");
	Shader terrain_shader("shaders/terrain_vert.glsl", "shaders/terrain_frag.glsl");
	//Shader sky_shader("shaders/sky_vert.glsl", "shaders/sky_frag.glsl");

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

		// GUI HERE
		// _______________ GUI _________________
		ImGui_ImplGlfw_NewFrame();
		{
			if (ImGui::BeginMainMenuBar()) {
				if (ImGui::BeginMenu("File")) {
					if (ImGui::MenuItem("Open")) {
						std::cout << "open ";

						if (ImGui::Begin("Input")) {
							ImGui::End();
						}
					}

					if (ImGui::MenuItem("Save")) {
						std::cout << "save ";
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Help")) {
					if (ImGui::MenuItem("No help for you!")) {
						std::cout << "help ";
					}

					ImGui::EndMenu();
				}

				ImGui::EndMainMenuBar();
			}
			// This creates a window
			ImGui::Begin("Window Title Here");
			ImGui::Text("Hello, world!");
			ImGui::End();
		}

		camera_movement();

		GL_calls();

		// Clear the colorbuffer
		glClearColor(0.9f, 0.0f, 0.0f, 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClear(GL_COLOR_BUFFER_BIT);

		// Create camera and projection
		glm::mat4 view, projection, model;
		view = camera.GetViewMatrix();
		projection = glm::perspective(camera.zoom, (float)WIDTH / (float)HEIGHT, 0.1f, 1000.0f);
		

		// ImGui functions end here
		//ImGui::Render();

		/*if (draw_wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); */

		//GL_calls();

		GLint Loc_model, Loc_view, Loc_projection;

		// ______________ Planet Geometry ________________
/*		terrain_shader.Use();

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
		*/
		glUseProgram(0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// Render ImGUI
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		ImGui::Render();

		// Swap the buffers
		glfwSwapBuffers(window);
	}
	// Properly de-allocate all resources once they've outlived their purpose
	//delete background_plane;
	for (int i = 0; i < skybox.size(); ++i)
		delete skybox[i];

	ImGui_ImplGlfw_Shutdown();
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

void draw_GUI() {

	ImGui_ImplGlfw_NewFrame();
	{
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Open")) {
					std::cout << "open ";

					if (ImGui::Begin("Input")) {
						ImGui::End();
					}
				}

				if (ImGui::MenuItem("Save")) {
					std::cout << "save ";
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Help")) {
				if (ImGui::MenuItem("No help for you!")) {
					std::cout << "help ";
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		ImGui::Text("Procedural Planet Maker");
		ImGui::Separator();

		ImGui::Text("Geometry");

		if (ImGui::SliderInt("Segments", &segments, 1, 200)) {
			delete sphere;
			sphere = new Sphere(0.0f, 0.0f, 0.0f, 1.0f, segments);
		}
		if (show_tooltips && ImGui::IsItemHovered())
			ImGui::SetTooltip("The numbers of segment the mesh has.");

		ImGui::SliderInt("Octaves", &octaves, 1, 10);
		if (show_tooltips && ImGui::IsItemHovered())
			ImGui::SetTooltip("The numbers of sub-step iterations the procedural method has.");

		ImGui::SliderInt("Seed", &seed, 0, 100);
		if (show_tooltips && ImGui::IsItemHovered())
			ImGui::SetTooltip("Change seed to vary the noise.");

		ImGui::SliderFloat("Lacunarity", &lacunarity, 0.0f, 1.0f);
		ImGui::SliderFloat("Vertex Frequency", &vert_frequency, 0.1f, 10.0f);
		if (show_tooltips && ImGui::IsItemHovered())
			ImGui::SetTooltip("Frequency of the noise.");

		ImGui::SliderFloat("Radius", &radius, 0.0f, 1.0f);
		if (show_tooltips && ImGui::IsItemHovered())
			ImGui::SetTooltip("Radius of the planet.");

		ImGui::SliderFloat("Elevation", &elevation, 0.0f, 0.2f);
		if (show_tooltips && ImGui::IsItemHovered())
			ImGui::SetTooltip("Maximum height of the mountains.");

		ImGui::Separator();

		if (ImGui::BeginMenu("Colors")) {

			ImGui::Text("Colors");
			ImGui::SliderFloat("Color Frequency", &frag_frequency, 0.1f, 10.0f);
			if (show_tooltips && ImGui::IsItemHovered())
				ImGui::SetTooltip("Frequency of the noise.");

			ImGui::Spacing();
			ImGui::ColorEdit3("Low color 1", color_water_1);
			ImGui::ColorEdit3("Low color 2", color_water_2);
			ImGui::ColorEdit3("Medium color 1", color_ground_1);
			ImGui::ColorEdit3("Medium color 2", color_ground_2);
			ImGui::ColorEdit3("High color 1", color_mountain_1);
			ImGui::ColorEdit3("High color 2", color_mountain_2);

			ImGui::EndMenu();
		}

		ImGui::Separator();

		if (ImGui::BeginMenu("Clouds")) {

			ImGui::Text("Clouds");
			ImGui::Checkbox("Enable clouds", &enable_sky);
			ImGui::SliderInt("Octaves", &sky_octaves, 1, 10);
			ImGui::SliderFloat("Frequency", &sky_frequency, 0.01f, 10.0f);
			ImGui::SliderInt("Seed", &sky_seed, 0, 10000);
			ImGui::SliderFloat("Speed", &sky_speed, 0.0f, 2.0f);

			ImGui::EndMenu();
		}

		ImGui::Separator();

		if (ImGui::BeginMenu("Light")) {
			ImGui::Text("Light options");
			ImGui::DragFloat3("Position", light_position, 0.01f, -3.0f, 3.0f);
			ImGui::DragFloat("Intensity", &light_intensity, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Shininess", &shininess, 0.01f, 0.01f, 10.0f);

			ImGui::EndMenu();
		}

		ImGui::Separator();

		if (ImGui::BeginMenu("Procedural method")) {
			if (ImGui::Checkbox("Perlin Noise", &use_perlin)) {
				use_simplex = use_cell = false;
			}
			if (ImGui::Checkbox("Simplex Noise", &use_simplex)) {
				use_perlin = use_cell = false;
			}
			if (ImGui::Checkbox("Cell Noise", &use_cell)) {
				use_simplex = use_perlin = false;
			}
			ImGui::EndMenu();
		}

		ImGui::Separator();
		ImGui::Text("Rotation");
		ImGui::SliderFloat("Azimuth", &rotation_degrees[1], 0.0f, 360.0f);
		ImGui::SliderFloat("Inclination", &rotation_degrees[0], 0.0f, 360.0f);

		rotation_radians[0] = rotation_degrees[0] * DEGREE_TO_RADIAN;
		rotation_radians[1] = rotation_degrees[1] * DEGREE_TO_RADIAN;

		ImGui::Spacing();
		ImGui::Checkbox("Show tooltips", &show_tooltips);

		if (ImGui::Button("Reset"))
		{
			radius = 1.0f;
			lacunarity = 1.0f;
			octaves = 6;
			seed = 0;

			color_water_1[0] = color_water_1[1] = color_water_1[2] = 1.0f;
			color_water_2[0] = color_water_2[1] = color_water_2[2] = 1.0f;
			color_ground_1[0] = color_ground_1[1] = color_ground_1[2] = 1.0f;
			color_ground_2[0] = color_ground_2[1] = color_ground_2[2] = 1.0f;
			color_mountain_1[0] = color_mountain_1[1] = color_mountain_1[2] = 1.0f;
			color_mountain_2[0] = color_mountain_2[1] = color_mountain_2[2] = 1.0f;

			use_perlin = true;
			rotation_degrees[0] = rotation_degrees[1] = 0.0f;
		}

		ImGui::Checkbox("Draw wireframe", &draw_wireframe);

		/*if (ImGui::Button("Reload shaders")) {
			shader.createShader("shaders/vert.glsl", "shaders/frag.glsl");
		}*/

		if (ImGui::BeginMenu("Load/Save")) {

			ImGui::InputText("<-", load_buffer, sizeof(load_buffer));

			ImGui::SameLine();
			if (ImGui::Button("Load")) {
				std::string asses(load_buffer);
				load_file(std::string(load_buffer));
			}

			ImGui::InputText("->", save_buffer, sizeof(save_buffer));

			ImGui::SameLine();
			if (ImGui::Button("Save")) {
				std::cout << "save ";
				save_file(std::string(save_buffer));
			}
			ImGui::InputTextMultiline("  ", files_buffer, sizeof(files_buffer));
			if (ImGui::Button("List all files")) {
				list_files();
			}

			ImGui::EndMenu();
		}
	}
}