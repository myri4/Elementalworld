#include "Application.hpp"

//DANGEROUS!
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image/stb_write.h>

wc::Application app;

int main() {
	wc::Log::Init();
	glfwInit();
	
	app.Start();
	wc::window.Destroy();
	glfwTerminate();

	return 0;
}