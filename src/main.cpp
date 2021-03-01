#include "Application.hpp"

wc::Application app;

int main() {
	wc::Log::Init();
	app.Start();

	glfwTerminate();
	return 0;
}