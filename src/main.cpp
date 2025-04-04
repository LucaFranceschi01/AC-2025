#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream> // to output

// IMGUI
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "ImGuizmo.h"

#include "framework/application.h"

// Globals
Application* app;

void render_gui(GLFWwindow* window, Application* app) 
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
    
	assert(window);

	{
		ImGui::Begin("Controls");

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		if (ImGui::TreeNode("Debugger")) {
			ImGui::Checkbox("View wireframe", &app->flag_wireframe);
			ImGui::Checkbox("View grid", &app->flag_grid);
			if (ImGui::IsMousePosValid())
				ImGui::Text("Mouse pos: (%g, %g)", xpos, ypos);
			else
				ImGui::Text("Mouse pos: <INVALID>");
			ImGui::Text("Mouse down:");
			for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) if (ImGui::IsMouseDown(i)) { ImGui::SameLine(); ImGui::Text("b%d (%.02f secs)", i, io.MouseDownDuration[i]); }
			ImGui::TreePop();
		}

		app->render_gui();

		ImGui::End();
	}

	// Rendering
	ImGui::Render();
	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void on_key_event(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (app != 0) {
		// Forward input data to ImGui
		ImGuiIO& io = ImGui::GetIO();

		// Do only if is not interacting with the ImGui
		if (!io.WantCaptureKeyboard) {
			if (action == GLFW_PRESS) {
				app->on_key_down(key, scancode);
			}
			if (action == GLFW_RELEASE) {
				app->on_key_up(key, scancode);
			}
		}
	}
}

void on_mouse_event(GLFWwindow* window, int button, int action, int mods)
{
	if (app != 0) {
		// Forward input data to ImGui
		ImGuiIO& io = ImGui::GetIO();

		// Do only if is not interacting with the ImGui
		if (!io.WantCaptureMouse) {
			if (button == GLFW_MOUSE_BUTTON_RIGHT) {
				if (action == GLFW_PRESS)
					app->on_right_mouse_down();
				else if (action == GLFW_RELEASE) {
					app->on_right_mouse_up();
				}
			}
			if (button == GLFW_MOUSE_BUTTON_LEFT) {
				if (action == GLFW_PRESS)
					app->on_left_mouse_down();
				else if (action == GLFW_RELEASE) {
					app->on_left_mouse_up();
				}
			}
			if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
				if (action == GLFW_PRESS)
					app->on_middle_mouse_down();
				else if (action == GLFW_RELEASE) {
					app->on_middle_mouse_up();
				}
			}
		}
	}
}

void on_cursor_event(GLFWwindow* window, double xpos, double ypos)
{
	if (app != 0) {
		// Forward input data to ImGui
		ImGuiIO& io = ImGui::GetIO();
		app->on_mouse_position(xpos, ypos);
	}
}


void on_scroll_event(GLFWwindow* window, double xoffset, double yoffset)
{
	if (app != 0) {
		// Forward input data to ImGui
		ImGuiIO& io = ImGui::GetIO();

		// Do only if is not interacting with the ImGui
		if (!io.WantCaptureMouse) {
			app->on_scroll(xoffset, yoffset);
		}
	}
}

void main_loop(GLFWwindow* window)
{
	int32_t width, height;
	glfwGetFramebufferSize(window, &width, &height);
	double prev_frame_time = 0.0;
	double xpos, ypos; // mouse position vars

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);

		// Poll for and process events
		glfwPollEvents();
		glfwGetCursorPos(window, &xpos, &ypos);
		app->mouse_position.x = static_cast<float>(xpos);
		app->mouse_position.y = static_cast<float>(ypos);

		double curr_time = glfwGetTime();
		double delta_time = curr_time - prev_frame_time;
		prev_frame_time = curr_time;
		app->update(delta_time);

		if (app->close) break;

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGuizmo::SetOrthographic(false);
		ImGuizmo::BeginFrame();

		//ImGui::ShowDemoWindow();

		app->render();

		render_gui(window, app);
		
		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		ImGui::EndFrame();
	}
}

int main(void) 
{
	/* Glfw (Window API) */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	GLFWwindow* window = glfwCreateWindow(1600, 900, "Computer Animation", nullptr, nullptr); // 1600, 900 or 1280, 720
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	/* Glew (OpenGL API) */
	if (glewInit() != GLEW_OK)
		std::cout << "[Error] GLEW not initialized" << std::endl;
	fprintf(stdout, "[INFO] Using GLEW %s\n", glewGetString(GLEW_VERSION));

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("\n[INFO] Renderer %s", renderer);
	printf("\n[INFO] OpenGL version supported %s\n\n", version);
	fflush(stdout);

	// Bind event callbacks
	glfwSetKeyCallback(window, on_key_event);
	glfwSetMouseButtonCallback(window, on_mouse_event);
	glfwSetCursorPosCallback(window, on_cursor_event);
	glfwSetScrollCallback(window, on_scroll_event);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
	// Setup Dear ImGui style
	ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	const char* glsl_version = "#version 330";
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	app = new Application();
	app->init(window);

	// Main loop, application gets inside here till user closes it
	main_loop(window);

	// Free memory
	delete app;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
