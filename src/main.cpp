//#define GLM_FORCE_INTRINSICS 
#define GLFW_INCLUDE_NONE
#include "Application.h"

//DANGEROUS!
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image/stb_write.h>
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

namespace wc {
	Application app;

	int main(int argc, char* argv[]) {
		wc::Log::Init();
		glfwSetErrorCallback([](int error, const char* description) {
			switch (error)
			{
			case GLFW_NOT_INITIALIZED:
				WC_ERROR("GLFW was not initialized! Description: {0}", description);
				break;
			case GLFW_NO_CURRENT_CONTEXT:
				WC_ERROR("There is no current GLFW context! Description: {0}", description);
				break;
			case GLFW_INVALID_ENUM:
				WC_ERROR("GLFW invalid enum! Description: {0}", description);
				break;
			case GLFW_INVALID_VALUE:
				WC_ERROR("GLFW invalid value! Description: {0}", description);
				break;
			case GLFW_OUT_OF_MEMORY:
				WC_ERROR("GLFW went out of memory! Description: {0}", description);
				break;
			case GLFW_API_UNAVAILABLE:
				WC_ERROR("GLFW API is not available! Description: {0}", description);
				break;
			case GLFW_VERSION_UNAVAILABLE:
				WC_ERROR("GLFW version is not available! Description: {0}", description);
				break;
			case GLFW_PLATFORM_ERROR:
				WC_ERROR("GLFW platform error! Description: {0}", description);
				break;
			case GLFW_FORMAT_UNAVAILABLE:
				WC_ERROR("GLFW format is not available! Description: {0}", description);
				break;
			}
			});
		glfwInit();
		if (glfwVulkanSupported()) {

			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			app.Start();
		}
		else 
			WC_ERROR("Vulkan driver is not supported!");
		glfwTerminate();

		return 0;
	}
}

int main(int argc, char* argv[]) {
	return wc::main(argc, argv);
}