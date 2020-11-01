#pragma once

#include <wclibs/Core.hpp>
#include "Lua.hpp"
#include <GLFW/glfw3.h>

enum class WindowStatus {
	OK, FAILED
};
class Window {
public:
	Window() {

	}
	Window(const char* name, uint32_t width, uint32_t height, uint32_t majorVer, uint32_t minorVer) {
		Create(name, width, height, majorVer, minorVer);
	}
	~Window() {
		glfwDestroyWindow(window);
	}
	WindowStatus Create(const char* name, uint32_t width, uint32_t height, uint32_t majorVer, uint32_t minorVer) {
		this->width = width;
		this->height = height;
		this->majorVersion = majorVer;
		this->minorVersion = minorVer;
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, majorVersion);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minorVersion);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
		window = glfwCreateWindow(width, height, name, nullptr, nullptr);

		if (window == nullptr) {
			glfwTerminate();
			return WindowStatus::FAILED;
		}
		return WindowStatus::OK;
		activateContext();
	}
	void activateContext() {
		glfwMakeContextCurrent(window);
		glfwSetFramebufferSizeCallback(window, Resize);
	}
	void loadFromFile(const char* file) {
		wc::Lua windowScript(file);
		float width, height;

		bool fullScreen = 0;
		bool vsync = 0;

		uint32 frameRateLimit = 0;

		fullScreen = windowScript.GetBool("fullscreen");
		if (fullScreen == true) {
			const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

			width = mode->width;
			height = mode->height;
			
		}
		else {
			width = (float)windowScript.GetNumber("screenWidth");
			height = (float)windowScript.GetNumber("screenHeight");
		}


		frameRateLimit = windowScript.GetNumber("framerateLimit");
		vsync = windowScript.GetBool("vsync");

		int32 nrComponents, imgWidth, imgHeight;
		stbi_set_flip_vertically_on_load(false);

		Create("Elementalworld", width, height, windowScript.GetNumber("majorVersion"), windowScript.GetNumber("minorVersion"));
		SetIcon(imgWidth, imgHeight, stbi_load(windowScript.GetString("iconPath"), &imgWidth, &imgHeight, &nrComponents, 0));
		SetFrameRateLimit(frameRateLimit);

	}
	void Display() {
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	void SetSize(unsigned int width, unsigned int height) {
		glfwSetWindowSize(window, width, height);
	}
	void setAntiAliasingLevel(uint32_t level) {
		glfwWindowHint(GLFW_SAMPLES, level);
	}
	void SetName(const char* name) {
	}
	void getWindowPos(int* width, int* height) {
		glfwGetWindowPos(window, width, height);
	}
	bool isOpen() {
		return !glfwWindowShouldClose(window);
	}
	void SetIcon(unsigned int width, unsigned int height, uint8_t* image) {
		GLFWimage icons[1];
		icons->pixels = image;
		icons->width = width;
		icons->height = height;
		glfwSetWindowIcon(window, 1, icons);
	}
	void SetFrameRateLimit(int limit) {
		glfwSwapInterval(limit);
	}
	uint32_t width, height;
	uint32_t majorVersion, minorVersion;
private:
	void Resize(GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);
	}
	GLFWwindow* window;
};