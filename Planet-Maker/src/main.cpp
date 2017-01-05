#include "GL/glew.h"
#include <imgui.h>
#include "imgui_impl_glfw.h"
#include "glfwContext.h"

#include "Shader.h"
#include "MatrixStack.h"
#include "Camera.h"
#include "Sphere.h"

void inputHandler(GLFWwindow* _window, double _dT);
void cameraHandler(GLFWwindow* _window, double _dT, Camera* _cam);
void GLcalls();

int main() {
	glfwContext glfw;
	GLFWwindow* currentWindow = nullptr;

	glfw.init(1920, 1080, "Procedural Planet Maker");
	glfw.getCurrentWindow(currentWindow);
	glfwSetInputMode(currentWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	glfwSetCursorPos(currentWindow, 960, 540);

	// Setup ImGui binding
	ImGui_ImplGlfw_Init(currentWindow, true);

	//start GLEW extension handler
	glewExperimental = GL_TRUE;
	GLenum l_GlewResult = glewInit();
	if (l_GlewResult != GLEW_OK)
		std::cout << "glewInit() error." << std::endl;


	// Print some info about the OpenGL context...
	glfw.printGLInfo();

	Shader sceneLight;
	sceneLight.createShader("shaders/scene.vert", "shaders/scene.frag");

	GLint locationP = glGetUniformLocation(sceneLight.programID, "P"); //perspective matrix
	GLint locationMV = glGetUniformLocation(sceneLight.programID, "MV"); //modelview matrix
	GLint locationColor = glGetUniformLocation(sceneLight.programID, "Color");

	MatrixStack MVstack; MVstack.init();

	Sphere sphere(0.0f, 0.0f, 0.0f, 1.0f);

	Camera mCamera;
	mCamera.setPosition(&glm::vec3(0.f, 0.f, 1.5f));
	mCamera.update();

	// Time related variables
	bool is_paused = false;
	bool fpsResetBool = false;

	double lastTime = glfwGetTime() - 0.001;
	double dT = 0.0;

	while (!glfwWindowShouldClose(currentWindow))
	{
		glfwPollEvents();

		ImGui_ImplGlfw_NewFrame();
		{
			ImGui::Text("Number of Particles");
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
				glfwSetCursorPos(currentWindow, 960, 540);
			}

			mCamera.fpsCamera(currentWindow, dT);
		}
		else
		{
			fpsResetBool = false;
		}

		GLcalls();

		glUseProgram(sceneLight.programID);

		MVstack.push();//Camera transforms --<

		glUniformMatrix4fv(locationP, 1, GL_FALSE, mCamera.getPerspective());
		MVstack.multiply(mCamera.getTransformM());

		MVstack.push();
		float color[] = { 1.f, 1.0f, 1.f, 1.f };
		glUniform4fv(locationColor, 1, &color[0]);
		MVstack.translate(sphere.getPosition());
		glUniformMatrix4fv(locationMV, 1, GL_FALSE, MVstack.getCurrentMatrix());
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