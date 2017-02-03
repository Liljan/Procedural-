#include "GL/glew.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include "glfwContext.h"
#include <iostream>
#include <fstream>

#include <Windows.h>
#include <string>
#include <sstream>

#include "Shader.h"
#include "MatrixStack.h"
#include "Camera.h"
#include "Sphere.h"
#include "CustomPlane.h"

void input_handler(GLFWwindow* _window, double _dT);
// void camera_handler(GLFWwindow* _window, double _dT, Camera* _cam);
void GL_calls();

static const float M_PI = 3.141592653f;
static const float DEGREE_TO_RADIAN = M_PI / 180.0f;
static const float RADIAN_TO_DEGREE = 180.0f / M_PI;
static const char file_name_buffer[30] = {};

static char load_buffer[256] = "";
static char save_buffer[256] = "";
static char files_buffer[1024] = "";

static const std::string FILE_ENDING = ".ass";

static glm::vec3* background_pos = new glm::vec3(0.0f, 0.0f, 3.0f);

int noise_method = 0;
bool use_perlin = true;
bool use_simplex = false;
bool use_worley = false;

// ________ Ocean _________

bool ocean_enabled = true;
float ocean_frequency = 4.0f;
int ocean_seed = 0;
int ocean_octaves = 6;
float ocean_color_1[3] = { 0.0f,0.352941f,1.0f };
float ocean_color_2[3] = { 0.0f,0.231142f,0.654902f };

// ________ TERRAIN _________
// Procedural related variables
int segments = 100;
float elevation = 0.1f;
float radius = 0.01f;
float vert_frequency = 4.0f;
float frag_frequency = 4.0f;
int octaves = 6;
int seed = 0;

float color_deep[3] = { 0.3f,0.3f,0.3f };
float color_beach[3] = { 1.0f,0.96f,0.57f };
float color_grass[3] = { 0.0535179f,0.454902f,0.0912952f };
float color_rock[3] = { 0.286275f,0.286275f,0.286275f };
float color_snow[3] = { 0.980392f,0.980392f,0.980392f };

// ________ SKY _________
// Procedural related variables
bool sky_enabled = true;

float sky_frequency = 4.0f;
int sky_seed = 0;
int sky_octaves = 6;
float sky_color[3] = { 1.0f,1.0f,1.0f };

Sphere* ocean_sphere;
Sphere* terrain_sphere;
Sphere* sky_sphere;

std::vector<CustomPlane*> skybox;
float scale = 100.0f;

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
		infile >> vert_frequency;
		infile >> octaves;
		infile >> seed;

		// colors
		file_to_color(infile, color_deep);
		file_to_color(infile, color_beach);
		file_to_color(infile, color_grass);
		file_to_color(infile, color_rock);
		file_to_color(infile, color_snow);

		infile.close();
		delete terrain_sphere;
		terrain_sphere = new Sphere(0.0f, 0.0f, 0.0f, 1.0f, segments);
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
		outfile << vert_frequency << std::endl;
		outfile << octaves << std::endl;
		outfile << seed << std::endl;

		// colors
		color_to_file(outfile, color_deep);
		color_to_file(outfile, color_beach);
		color_to_file(outfile, color_grass);
		color_to_file(outfile, color_rock);
		color_to_file(outfile, color_snow);

		outfile.close();
	}
	catch (const std::exception&)
	{
		std::cout << "Error saving" << std::endl;
	}
}


inline float degree_to_radians(float degree) {
	return M_PI*degree / 180.0f;
}


int main() {
	glfwContext glfw;
	GLFWwindow* currentWindow = nullptr;

	glfw.init(1920, 1080, "Procedural Planet Maker");
	glfw.getCurrentWindow(currentWindow);
	glfwSetInputMode(currentWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	glfwSetCursorPos(currentWindow, 1920 / 2, 1080 / 2);

	// Setup ImGui binding
	ImGui_ImplGlfw_Init(currentWindow, true);

	//start GLEW extension handler
	glewExperimental = GL_TRUE;
	GLenum l_GlewResult = glewInit();
	if (l_GlewResult != GLEW_OK)
		std::cout << "glewInit() error." << std::endl;

	// Print some info about the OpenGL context...
	glfw.printGLInfo();

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
	bool FPS_reset = false;

	double last_time = glfwGetTime();
	double delta_time = 0.0;

	// other shite
	float rotation_degrees[2] = { 0.0f,0.0f };
	float rotation_radians[2] = { 0.0f,0.0f };

	// __________ TERRAIN ______________
	Shader terrain_shader;
	terrain_shader.createShader("shaders/terrain_vert.glsl", "shaders/terrain_frag.glsl");

	GLint loc_P_terrain = glGetUniformLocation(terrain_shader.programID, "P"); // perspective matrix
	GLint loc_V_terrain = glGetUniformLocation(terrain_shader.programID, "V"); // modelview matrix
	GLint loc_M_terrain = glGetUniformLocation(terrain_shader.programID, "M"); // modelview matrix

	GLint loc_color_deep = glGetUniformLocation(terrain_shader.programID, "color_deep"); // water color of the planet
	GLint loc_color_beach = glGetUniformLocation(terrain_shader.programID, "color_beach"); // ground color of the planet
	GLint loc_color_grass = glGetUniformLocation(terrain_shader.programID, "color_grass"); // ground color of the planet
	GLint loc_color_rock = glGetUniformLocation(terrain_shader.programID, "color_rock"); // mountain color of the planet
	GLint loc_color_snow = glGetUniformLocation(terrain_shader.programID, "color_snow"); // mountain color of the planet

	GLint loc_terrain_method = glGetUniformLocation(terrain_shader.programID, "noise_method");

	GLfloat loc_radius = glGetUniformLocation(terrain_shader.programID, "radius");
	GLfloat loc_elevation = glGetUniformLocation(terrain_shader.programID, "elevationModifier");
	GLint loc_seed = glGetUniformLocation(terrain_shader.programID, "seed");
	GLint loc_octaves = glGetUniformLocation(terrain_shader.programID, "octaves");
	GLfloat loc_vert_frequency = glGetUniformLocation(terrain_shader.programID, "vert_frequency");
	GLfloat loc_frag_frequency = glGetUniformLocation(terrain_shader.programID, "frag_frequency");

	// __________ SKY ______________

	Shader sky_shader;
	sky_shader.createShader("shaders/sky_vert.glsl", "shaders/sky_frag.glsl");

	GLint loc_P_sky = glGetUniformLocation(sky_shader.programID, "P"); // perspective matrix
	GLint loc_V_sky = glGetUniformLocation(sky_shader.programID, "V"); // view matrix
	GLint loc_M_sky = glGetUniformLocation(sky_shader.programID, "M"); // model matrix

	GLint loc_sky_radius = glGetUniformLocation(sky_shader.programID, "radius");
	GLfloat loc_sky_elevation = glGetUniformLocation(sky_shader.programID, "elevationModifier");

	GLint loc_sky_method = glGetUniformLocation(sky_shader.programID, "noise_method");

	GLint loc_sky_time = glGetUniformLocation(sky_shader.programID, "time");
	GLint loc_sky_speed = glGetUniformLocation(sky_shader.programID, "speed");
	GLint loc_sky_frequency = glGetUniformLocation(sky_shader.programID, "frequency");
	GLint loc_sky_octaves = glGetUniformLocation(sky_shader.programID, "octaves");
	GLint loc_sky_seed = glGetUniformLocation(sky_shader.programID, "seed");
	GLint loc_sky_color = glGetUniformLocation(sky_shader.programID, "sky_color");

	// __________ OCEAN ______________

	Shader ocean_shader;
	ocean_shader.createShader("shaders/ocean_vert.glsl", "shaders/ocean_frag.glsl");

	GLint loc_P_ocean = glGetUniformLocation(ocean_shader.programID, "P"); // perspective matrix
	GLint loc_V_ocean = glGetUniformLocation(ocean_shader.programID, "V"); // modelview matrix
	GLint loc_M_ocean = glGetUniformLocation(ocean_shader.programID, "M"); // modelview matrix

	GLint loc_light_position = glGetUniformLocation(ocean_shader.programID, "light_pos");
	GLint loc_light_intensity = glGetUniformLocation(ocean_shader.programID, "light_intensity");
	GLint loc_shininess = glGetUniformLocation(ocean_shader.programID, "shininess");

	GLint loc_ocean_method = glGetUniformLocation(ocean_shader.programID, "noise_method");

	GLint loc_ocean_radius = glGetUniformLocation(ocean_shader.programID, "radius");
	GLfloat loc_ocean_elevation = glGetUniformLocation(ocean_shader.programID, "elevationModifier");
	GLint loc_ocean_frequency = glGetUniformLocation(ocean_shader.programID, "frequency");
	GLint loc_ocean_octaves = glGetUniformLocation(ocean_shader.programID, "octaves");
	GLint loc_ocean_seed = glGetUniformLocation(ocean_shader.programID, "seed");
	GLint loc_ocean_color_1 = glGetUniformLocation(ocean_shader.programID, "color_1");
	GLint loc_ocean_color_2 = glGetUniformLocation(ocean_shader.programID, "color_2");

	// __________ STAR BACKGROUND ______________

	Shader stars_shader;
	stars_shader.createShader("shaders/star_vert.glsl", "shaders/star_frag.glsl");

	skybox.push_back(new CustomPlane(scale, glm::vec3(0, 0, scale / 2.f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f))); // back
	skybox.push_back(new CustomPlane(scale, glm::vec3(0, 0, -scale / 2.f), M_PI, glm::vec3(1.0f, 0.0f, 0.0f))); // front
	skybox.push_back(new CustomPlane(scale, glm::vec3(0, scale / 2.f, 0.0f), -M_PI / 2.f, glm::vec3(1.0f, 0.0f, 0.0f))); // top
	skybox.push_back(new CustomPlane(scale, glm::vec3(0, -scale / 2.f, 0.0f), M_PI / 2.f, glm::vec3(1.0f, 0.0f, 0.0f))); // bottom
	skybox.push_back(new CustomPlane(scale, glm::vec3(scale / 2.f, 0.0f, 0.0f), M_PI / 2.f, glm::vec3(0.0f, 1.0f, 0.0f))); // right
	skybox.push_back(new CustomPlane(scale, glm::vec3(-scale / 2.f, 0.0f, 0.0f), -M_PI / 2.f, glm::vec3(0.0f, 1.0f, 0.0f))); // left

	GLint loc_P_stars = glGetUniformLocation(stars_shader.programID, "P"); // perspective matrix
	GLint loc_V_stars = glGetUniformLocation(stars_shader.programID, "V"); // view matrix
	GLint loc_M_stars = glGetUniformLocation(stars_shader.programID, "M"); // model matrix

	// _____________________________________________________

	terrain_sphere = new Sphere(0.0f, 0.0f, 0.0f, 1.0f, segments);
	sky_sphere = new Sphere(0.0f, 0.0f, 0.0f, 1.0f, 32);
	ocean_sphere = new Sphere(0.0f, 0.0f, 0.0f, 1.0f, 32);

	Camera camera;
	camera.setPosition(&glm::vec3(0.f, 0.f, 3.0f));
	camera.update();

	// RENDER LOOP \__________________________________________/

	while (!glfwWindowShouldClose(currentWindow))
	{
		glfwPollEvents();

		ImGui_ImplGlfw_NewFrame();
		{
			ImGui::Text("Procedural Planet Maker");
			ImGui::Separator();
			ImGui::Text("Use CTRL + W,A,S,D to move camera \nfreely.");
			ImGui::Separator();

			ImGui::Text("Geometry");

			if (ImGui::SliderInt("Segments", &segments, 1, 200)) {
				delete terrain_sphere;
				terrain_sphere = new Sphere(0.0f, 0.0f, 0.0f, 1.0f, segments);
			}
			if (show_tooltips && ImGui::IsItemHovered())
				ImGui::SetTooltip("The numbers of segment the mesh has.");

			ImGui::SliderInt("Octaves", &octaves, 1, 10);
			if (show_tooltips && ImGui::IsItemHovered())
				ImGui::SetTooltip("The numbers of sub-step iterations the procedural method has.");

			ImGui::SliderInt("Seed", &seed, 0, 100);
			if (show_tooltips && ImGui::IsItemHovered())
				ImGui::SetTooltip("Change seed to vary the noise.");

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
				ImGui::ColorEdit3("Deep color", color_deep);
				ImGui::ColorEdit3("Beach color", color_beach);
				ImGui::ColorEdit3("Grass color", color_grass);
				ImGui::ColorEdit3("Mountain color", color_rock);
				ImGui::ColorEdit3("Snow color", color_snow);

				ImGui::EndMenu();
			}

			ImGui::Separator();

			if (ImGui::BeginMenu("Clouds")) {

				ImGui::Text("Clouds");
				ImGui::Checkbox("Enable clouds", &sky_enabled);
				ImGui::ColorEdit3("Color", sky_color);
				ImGui::SliderInt("Octaves", &sky_octaves, 1, 10);
				ImGui::SliderFloat("Frequency", &sky_frequency, 0.01f, 10.0f);
				ImGui::SliderInt("Seed", &sky_seed, 0, 10000);
				ImGui::SliderFloat("Speed", &sky_speed, 0.0f, 2.0f);

				ImGui::EndMenu();
			}

			ImGui::Separator();

			if (ImGui::BeginMenu("Ocean")) {

				ImGui::Text("Ocean");
				ImGui::Checkbox("Enable ocean", &ocean_enabled);
				ImGui::ColorEdit3("Color 1", ocean_color_1);
				ImGui::ColorEdit3("Color 2", ocean_color_2);
				ImGui::SliderInt("Octaves", &ocean_octaves, 1, 10);
				ImGui::SliderFloat("Frequency", &ocean_frequency, 0.01f, 10.0f);
				ImGui::SliderInt("Seed", &ocean_seed, 0, 10000);

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
					use_simplex = use_worley = false;
					noise_method = 0;
				}
				if (ImGui::Checkbox("Simplex Noise", &use_simplex)) {
					use_perlin = use_worley = false;
					noise_method = 1;
				}
				if (ImGui::Checkbox("Cell Noise", &use_worley)) {
					use_simplex = use_perlin = false;
					noise_method = 2;
				}
				ImGui::EndMenu();
			}

			ImGui::Separator();
			ImGui::Text("Rotation");
			ImGui::SliderFloat("Inclination", &rotation_degrees[1], 0.0f, 360.0f);
			ImGui::SliderFloat("Azimuth", &rotation_degrees[0], 0.0f, 360.0f);

			rotation_radians[0] = degree_to_radians(rotation_degrees[0]);
			rotation_radians[1] = degree_to_radians(rotation_degrees[1]);

			ImGui::Spacing();
			ImGui::Checkbox("Show tooltips", &show_tooltips);

			if (ImGui::Button("Reset")) {
				radius = 1.0f;
				octaves = 6;
				seed = 0;

				color_deep[0] = color_deep[1] = color_deep[2] = 1.0f;
				color_beach[0] = color_beach[1] = color_beach[2] = 1.0f;
				color_grass[0] = color_grass[1] = color_grass[2] = 1.0f;
				color_rock[0] = color_rock[1] = color_rock[2] = 1.0f;
				color_snow[0] = color_snow[1] = color_snow[2] = 1.0f;

				use_perlin = true;
				rotation_degrees[0] = rotation_degrees[1] = 0.0f;
			}

			ImGui::Checkbox("Draw wireframe", &draw_wireframe);

			if (ImGui::Button("Reload shaders")) {
				sky_shader.createShader("shaders/ocean_vert.glsl", "shaders/ocean_frag.glsl");
				terrain_shader.createShader("shaders/vert.glsl", "shaders/frag.glsl");
				sky_shader.createShader("shaders/sky_vert.glsl", "shaders/sky_frag.glsl");
				stars_shader.createShader("shaders/star_vert.glsl", "shaders/star_frag.glsl");
			}

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

		delta_time = glfwGetTime() - last_time;
		last_time = glfwGetTime();

		//glfw input handler
		input_handler(currentWindow, delta_time);

		if (glfwGetKey(currentWindow, GLFW_KEY_LEFT_CONTROL))
		{
			if (!FPS_reset)
			{
				FPS_reset = true;
				glfwSetCursorPos(currentWindow, 1920 / 2, 1080 / 2);
			}

			camera.fpsCamera(currentWindow, delta_time);
		}
		else
		{
			FPS_reset = false;
		}

		// __________ RENDERING SETTINGS _______

		GL_calls();

		if (draw_wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		// STARS SHADER
		glUseProgram(stars_shader.programID);
		glUniformMatrix4fv(loc_P_stars, 1, GL_FALSE, camera.getPerspective());
		glUniformMatrix4fv(loc_V_stars, 1, GL_FALSE, camera.getTransformF());

		for (int i = 0; i < skybox.size(); ++i) {
			glm::mat4 model;
			model = glm::translate(model, skybox[i]->m_position);
			model = glm::rotate(model, skybox[i]->m_angle, skybox[i]->m_rotation);

			glUniformMatrix4fv(loc_M_stars, 1, GL_FALSE, glm::value_ptr(model));
			skybox[i]->render();
		}

		// _________ PLANET __________
		glUseProgram(terrain_shader.programID);

		glUniformMatrix4fv(loc_P_terrain, 1, GL_FALSE, camera.getPerspective());
		glUniformMatrix4fv(loc_V_terrain, 1, GL_FALSE, camera.getTransformF());

		glm::mat4 model;
		model = glm::rotate(model, rotation_radians[0], glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, rotation_radians[1], glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, *sky_sphere->getPosition());
		glUniformMatrix4fv(loc_M_terrain, 1, GL_FALSE, glm::value_ptr(model));

		glUniform1i(loc_terrain_method, noise_method);

		glUniform1f(loc_radius, radius);
		glUniform1f(loc_elevation, elevation);
		glUniform1i(loc_seed, seed);
		glUniform1i(loc_octaves, octaves);
		glUniform1f(loc_vert_frequency, vert_frequency);
		glUniform1f(loc_frag_frequency, frag_frequency);

		glUniform3fv(loc_color_deep, 1, &color_deep[0]);
		glUniform3fv(loc_color_beach, 1, &color_beach[0]);
		glUniform3fv(loc_color_grass, 1, &color_grass[0]);
		glUniform3fv(loc_color_rock, 1, &color_rock[0]);
		glUniform3fv(loc_color_snow, 1, &color_snow[0]);

		terrain_sphere->render();

		// OCEAN SHADER
		if (ocean_enabled) {
			glUseProgram(ocean_shader.programID);

			glUniformMatrix4fv(loc_P_ocean, 1, GL_FALSE, camera.getPerspective());
			glUniformMatrix4fv(loc_V_ocean, 1, GL_FALSE, camera.getTransformF());

			glm::mat4 model;
			model = glm::rotate(model, rotation_radians[0], glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::rotate(model, rotation_radians[1], glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::translate(model, *ocean_sphere->getPosition());
			glUniformMatrix4fv(loc_M_ocean, 1, GL_FALSE, glm::value_ptr(model));

			glUniform1i(loc_ocean_method, noise_method);

			glUniform3fv(loc_light_position, 1, &light_position[0]);
			glUniform1f(loc_light_intensity, light_intensity);
			glUniform1f(loc_shininess, shininess);

			glUniform1f(loc_ocean_radius, radius);
			glUniform1f(loc_ocean_elevation, elevation);
			glUniform1i(loc_ocean_seed, ocean_seed);
			glUniform1f(loc_ocean_frequency, ocean_frequency);
			glUniform1i(loc_ocean_octaves, ocean_octaves);
			glUniform3fv(loc_ocean_color_1, 1, &ocean_color_1[0]);
			glUniform3fv(loc_ocean_color_2, 1, &ocean_color_2[0]);

			ocean_sphere->render();
		}

		// SKY SHADER
		if (sky_enabled) {
			glUseProgram(sky_shader.programID);

			glUniformMatrix4fv(loc_P_sky, 1, GL_FALSE, camera.getPerspective());
			glUniformMatrix4fv(loc_V_sky, 1, GL_FALSE, camera.getTransformF());

			glm::mat4 model;
			model = glm::rotate(model, rotation_radians[0], glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::rotate(model, rotation_radians[1], glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::translate(model, *sky_sphere->getPosition());
			glUniformMatrix4fv(loc_M_sky, 1, GL_FALSE, glm::value_ptr(model));

			glUniform1i(loc_sky_method, noise_method);

			// update time
			time = (float)glfwGetTime();
			glUniform1f(loc_sky_time, time);
			glUniform1f(loc_sky_speed, sky_speed);
			glUniform1f(loc_sky_radius, radius);
			glUniform1f(loc_sky_elevation, elevation);
			glUniform1i(loc_sky_seed, sky_seed);
			glUniform1f(loc_sky_frequency, sky_frequency);
			glUniform1i(loc_sky_octaves, sky_octaves);
			glUniform3fv(loc_sky_color, 1, &sky_color[0]);

			sky_sphere->render();
		}

		glUseProgram(0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// Rendering imgui
		int display_w, display_h;
		glfwGetFramebufferSize(currentWindow, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		ImGui::Render();

		glfwSwapBuffers(currentWindow);
	}

	ImGui_ImplGlfw_Shutdown();
	delete ocean_sphere;
	delete terrain_sphere;
	delete sky_sphere;

	return 0;
}


void input_handler(GLFWwindow* _window, double _dT)
{
	if (glfwGetKey(_window, GLFW_KEY_ESCAPE)) {
		glfwSetWindowShouldClose(_window, GL_TRUE);
	}
}


void GL_calls()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	glDisable(GL_TEXTURE);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

