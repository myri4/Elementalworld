#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <lua/lua.hpp>
#include <sol/sol.hpp>

namespace wc {
	void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }

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
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

			window = glfwCreateWindow(windowScript["screenWidth"], windowScript["screenHeight"], title, mode, nullptr);
			bool vsync = windowScript["vsync"];
			glfwMakeContextCurrent(window);
			if (!vsync)glfwSwapInterval(0);
			glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
		}

		void Destroy() const {
			glfwDestroyWindow(window);
		}

		void display() {
			float currentTime = glfwGetTime();

			if (currentTime - lastFrame >= 1.0 / framerateLimit || framerateLimit == 0)
			{
				lastFrame = currentTime;
				glfwSwapBuffers(window);
			}
			glfwPollEvents();
		}

		const glm::vec2& GetPos() const {
			int xpos, ypos;
			glfwGetWindowPos(window, &xpos, &ypos);
			return { xpos, ypos };
		}

		const glm::vec2& GetSize() const {
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

		void setFramerateLimit(const int& limit) {
			framerateLimit = limit;
		}
	private:
		float lastFrame = 0.0f;
		int framerateLimit = 0;
		GLFWwindow* window;
	};

}
#endif