#include "Application.hpp"

//DANGEROUS!
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image/stb_write.h>

wc::Application app;

void* operator new(size_t bytes) {
	heapMemory += bytes;
	return malloc(bytes);
}

void operator delete(void* memory, size_t bytes) {
	heapMemory -= bytes;
	free(memory);
}

void operator delete[](void* memory, size_t bytes) {
	heapMemory -= bytes;
	free(memory);
}

int main() {
	wc::Log::Init();
	glfwInit();

	app.Start();

	glfwTerminate();
	return 0;
}