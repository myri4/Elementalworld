#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <sol/sol.hpp>
#include "Log.hpp"

namespace wc {

	bool resized = false;

	double scrollX, scrollY;
	bool mouseScrolled = false;
	uint32_t currentKeyPressed = 0; 
	bool keyPressed = false;
	bool buttonPressed = false;
	int currKey;
	int Action;
	int mouseButton;
	int mouseAction;
	bool mouseUsed = false;

	class Window {
	public:
		Window() {}
		~Window() { Destroy(); }

		void Create(const char* luaScript, const char* title) {
			sol::state windowScript;
			windowScript.script_file(luaScript);

			GLFWmonitor* mode = nullptr;
			if (windowScript["fullscreen"]) mode = glfwGetPrimaryMonitor();

			window = glfwCreateWindow(windowScript["screenWidth"], windowScript["screenHeight"], title, mode, nullptr);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			glfwWindowHint(GLFW_SAMPLES, windowScript["antialiasingLevel"]);
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

			bool vsync = windowScript["vsync"];
			glfwMakeContextCurrent(window);
			if (!vsync) glfwSwapInterval(0);
			glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {glViewport(0, 0, width, height); resized = true; });
			glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) { scrollX = xoffset; scrollY = yoffset; mouseScrolled = true; });
			glfwSetCharCallback(window, [](GLFWwindow* window, uint32_t codepoint) { currentKeyPressed = codepoint; keyPressed = true; });
			glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
				currKey = key;
				Action = action;
				buttonPressed = true;
				});

			glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
				mouseUsed = true;
				mouseButton = button;
				mouseAction = action;
				});

			if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) WC_ERROR("Failed to initialize GLAD");
		}

		void Destroy() const {
			glfwDestroyWindow(window);
		}

		void display() {
			resized = false;
			keyPressed = false;
			buttonPressed = false;
			mouseScrolled = false;
			mouseUsed = false;
			glfwSwapBuffers(window);
			glfwPollEvents();
		}

		const char* getClipboard() {
			return glfwGetClipboardString(window);
		}

		void setClipboard(const char* string) {
			glfwSetClipboardString(window, string);
		}

		glm::ivec2 GetPos() const {
			int xpos, ypos;
			glfwGetWindowPos(window, &xpos, &ypos);
			return { xpos, ypos };
		}

		auto getKey(int key){
			return glfwGetKey(window, key);
		}

		glm::ivec2 GetSize() const {
			int width, height;
			glfwGetWindowSize(window, &width, &height);
			return { width, height };
		}

		void close() const {
			glfwSetWindowShouldClose(window, true);
		}

		bool isOpen() const {
			return !glfwWindowShouldClose(window);
		}

		bool hasFocus() const {
			return glfwGetWindowAttrib(window, GLFW_FOCUSED);
		}

		void setActive() const {
			glfwMakeContextCurrent(window);
		}
	private:
		GLFWwindow* window = nullptr;
	};
}