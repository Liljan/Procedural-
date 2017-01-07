#include "GL/glew.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include "glfwContext.h"
#include <Windows.h>

#include "Shader.h"
#include "MatrixStack.h"
#include "Camera.h"
#include "Sphere.h"

void inputHandler(GLFWwindow* _window, double _dT);
void cameraHandler(GLFWwindow* _window, double _dT, Camera* _cam);
void GLcalls();

static const float M_PI = 3.141592653f;

inline float degree_to_radians(float degree) {
	return M_PI*degree / 180.0f;
}

int main() {
	glfwContext glfw;
	GLFWwindow* currentWindow = nullptr;

	glfw.init(1280, 720, "Procedural Planet Maker");
	glfw.getCurrentWindow(currentWindow);
	glfwSetInputMode(currentWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	glfwSetCursorPos(currentWindow, 1280 / 2, 720 / 2);

	// Setup ImGui binding
	ImGui_ImplGlfw_Init(currentWindow, true);

	//start GLEW extension handler
	glewExperimental = GL_TRUE;
	GLenum l_GlewResult = glewInit();
	if (l_GlewResult != GLEW_OK)
		std::cout << "glewInit() error." << std::endl;

	// Print some info about the OpenGL context...
	glfw.printGLInfo();

	// GUI related variables
	bool show_tooltips = true;

	// Time related variables
	bool is_paused = false;
	bool fpsResetBool = false;

	double lastTime = glfwGetTime() - 0.001;
	double dT = 0.0;

	// Procedural related variables
	float elevation = 0.1f;
	float radius = 0.01f;
	float lacunarity = 1.0f;
	float frequency = 4.0f;
	int octaves = 6;
	int seed = 0;

	float color_glow[3] = { 1.0f,1.0f,0.0f };
	float color_low[3] = { 1.0f,0.1f,0.0f };
	float color_med[3] = { 0.30f,0.3f,0.30f };
	float color_high[3] = { 0.05f,0.01f,0.03f };

	bool use_perlin = true;
	bool use_simplex = false;
	bool use_cell = false;

	// other shite
	float rotation_degrees[3] = { 0.0f,0.0f, 0.0f };
	float rotation_radians[3] = { 0.0f,0.0f, 0.0f };

	Shader proceduralShader;
	proceduralShader.createShader("shaders/vert.glsl", "shaders/frag.glsl");

	GLint locationP = glGetUniformLocation(proceduralShader.programID, "P"); // perspective matrix
	GLint locationMV = glGetUniformLocation(proceduralShader.programID, "MV"); // modelview matrix

	GLint gl_color_glow = glGetUniformLocation(proceduralShader.programID, "color_glow"); // base color of planet
	GLint gl_color_low = glGetUniformLocation(proceduralShader.programID, "color_low"); // base color of planet
	GLint gl_color_med = glGetUniformLocation(proceduralShader.programID, "color_med"); // base color of planet
	GLint gl_color_high = glGetUniformLocation(proceduralShader.programID, "color_high"); // base color of planet

	GLfloat gl_radius = glGetUniformLocation(proceduralShader.programID, "radius");
	GLfloat gl_elevation = glGetUniformLocation(proceduralShader.programID, "elevationModifier");
	GLint gl_seed = glGetUniformLocation(proceduralShader.programID, "seed");
	GLint gl_octaves = glGetUniformLocation(proceduralShader.programID, "octaves");
	GLfloat gl_frequency = glGetUniformLocation(proceduralShader.programID, "frequency");

	MatrixStack MVstack; MVstack.init();

	Sphere sphere(0.0f, 0.0f, 0.0f, 1.0f, 32 * 4);

	Camera mCamera;
	mCamera.setPosition(&glm::vec3(0.f, 0.f, 3.0f));
	mCamera.update();

	// RENDER LOOP \__________________________________________/

	while (!glfwWindowShouldClose(currentWindow))
	{
		glfwPollEvents();

		ImGui_ImplGlfw_NewFrame();
		{
			ImGui::Text("Procedural Planet Maker");
			ImGui::Separator();
			ImGui::SliderInt("Octaves", &octaves, 1, 10);
			if (show_tooltips && ImGui::IsItemHovered())
				ImGui::SetTooltip("The numbers of sub-step iterations the procedural method has.");

			ImGui::SliderInt("Seed", &seed, 0, 100);
			if (show_tooltips && ImGui::IsItemHovered())
				ImGui::SetTooltip("Change seed to vary the noise.");

			ImGui::SliderFloat("Lacunarity", &lacunarity, 0.0f, 1.0f);
			ImGui::SliderFloat("Frequency", &frequency, 0.1f, 10.0f);
			if (show_tooltips && ImGui::IsItemHovered())
				ImGui::SetTooltip("Frequency of the noise.");

			ImGui::SliderFloat("Radius", &radius, 0.0f, 1.0f);
			if (show_tooltips && ImGui::IsItemHovered())
				ImGui::SetTooltip("Radius of the planet.");

			ImGui::SliderFloat("Elevation", &elevation, 0.0f, 0.2f);
			if (show_tooltips && ImGui::IsItemHovered())
				ImGui::SetTooltip("Maximum height of the mountains.");

			ImGui::Separator();

			ImGui::Text("Colors");
			ImGui::ColorEdit3("Glow", color_glow);
			ImGui::ColorEdit3("Low", color_low);
			ImGui::ColorEdit3("Medium", color_med);
			ImGui::ColorEdit3("High", color_high);

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

			ImGui::SliderFloat("Azimuth", &rotation_degrees[1], 0.0f, 360.0f);
			ImGui::SliderFloat("Inclination", &rotation_degrees[0], 0.0f, 360.0f);

			rotation_radians[0] = degree_to_radians(rotation_degrees[0]);
			rotation_radians[1] = degree_to_radians(rotation_degrees[1]);

			ImGui::Spacing();
			ImGui::Checkbox("Show tooltips", &show_tooltips);

			if (ImGui::Button("Reset")) {
				radius = 1.0f;
				lacunarity = 1.0f;
				octaves = 6;
				seed = 0;

				color_glow[0] = color_glow[1] = color_glow[2] = 1.0f;
				color_low[0] = color_low[1] = color_low[2] = 1.0f;
				color_med[0] = color_med[1] = color_med[2] = 1.0f;
				color_high[0] = color_high[1] = color_high[2] = 1.0f;

				use_perlin = true;
				rotation_degrees[0] = rotation_degrees[1] = rotation_degrees[2] = 0.0f;
			}

		}

		if (dT > 1.0 / 30.0)
		{
			lastTime = glfwGetTime();
		}
		else if (!is_paused)
		{
			dT = glfwGetTime() - lastTime;
		}

		//glfw input handler
		inputHandler(currentWindow, dT);

		if (glfwGetKey(currentWindow, GLFW_KEY_LEFT_CONTROL))
		{
			if (!fpsResetBool)
			{
				fpsResetBool = true;
				glfwSetCursorPos(currentWindow, 1280 / 2, 720 / 2);
			}

			mCamera.fpsCamera(currentWindow, dT);
		}
		else
		{
			fpsResetBool = false;
		}

		GLcalls();

		glUseProgram(proceduralShader.programID);

		MVstack.push();//Camera transforms --<

		glUniformMatrix4fv(locationP, 1, GL_FALSE, mCamera.getPerspective());
		MVstack.multiply(mCamera.getTransformM());

		MVstack.push();
		//glUniform4fv(gl_color_low, 1, &color_low[0]);
		MVstack.translate(sphere.getPosition());
		MVstack.rotX(rotation_radians[0]);
		MVstack.rotY(rotation_radians[1]);
		glUniformMatrix4fv(locationMV, 1, GL_FALSE, MVstack.getCurrentMatrix());

		glUniform1f(gl_radius, radius);
		glUniform1f(gl_elevation, elevation);
		glUniform1i(gl_seed, seed);
		glUniform1i(gl_octaves, octaves);
		glUniform1f(gl_frequency, frequency);

		glUniform3fv(gl_color_glow, 1, &color_glow[0]);
		glUniform3fv(gl_color_low, 1, &color_low[0]);
		glUniform3fv(gl_color_med, 1, &color_med[0]);
		glUniform3fv(gl_color_high, 1, &color_high[0]);

		sphere.render();
		MVstack.pop();

		MVstack.pop(); //Camera transforms >--

		glUseProgram(0);

		// Rendering imgui
		int display_w, display_h;
		glfwGetFramebufferSize(currentWindow, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		ImGui::Render();

		glfwSwapBuffers(currentWindow);
	}

	ImGui_ImplGlfw_Shutdown();

	return 0;
}


void inputHandler(GLFWwindow* _window, double _dT)
{
	if (glfwGetKey(_window, GLFW_KEY_ESCAPE)) {
		glfwSetWindowShouldClose(_window, GL_TRUE);
	}
}


void GLcalls()
{
	glClearColor(0.01f, 0.01f, 0.01f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glDisable(GL_TEXTURE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}