#ifndef WINDOW_HPP
#define WINDOW_HPP
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <lua/lua.hpp>
#include <sol/sol.hpp>

namespace wc {
	void framebuffer_size_callback(GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}
	class Window {
	public:
		Window() {}
		~Window() { Destroy(); }

		void Create(const char* luaScript, const char* title) {
			sol::state windowScript;
			windowScript.script_file(luaScript);

			GLFWmonitor* mode = nullptr;
			if (windowScript["fullscreen"])mode = glfwGetPrimaryMonitor();

			glfwInit();
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			glfwWindowHint(GLFW_SAMPLES, windowScript["antialiasingLevel"]);
			glfwWindowHint(GLFW_REFRESH_RATE, windowScript["framerateLimit"]);

			window = glfwCreateWindow(windowScript["screenWidth"], windowScript["screenHeight"], title, mode, nullptr);

			glfwMakeContextCurrent(window);
			glfwSwapInterval(windowScript["vsync"]);
			glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
		}

		void Destroy() {
			glfwDestroyWindow(window);
		}

		void display() {
			glfwSwapBuffers(window);
			glfwPollEvents();
		}

		const glm::vec2& GetPos() {
			int xpos, ypos;
			glfwGetWindowPos(window, &xpos, &ypos);
			return { xpos, ypos };
		}

		const glm::vec2& GetSize() {
			int width, height;
			glfwGetWindowSize(window, &width, &height);
			return { width, height };
		}

		void close() {
			glfwSetWindowShouldClose(window, true);
		}

		bool isOpen() {
			return !glfwWindowShouldClose(window);
		}

		bool hasFocus() {
			return glfwGetWindowAttrib(window, GLFW_FOCUSED);
		}

		void setActive() {
			glfwMakeContextCurrent(window);
		}
	private:

		GLFWwindow* window;
	};

}
#endif