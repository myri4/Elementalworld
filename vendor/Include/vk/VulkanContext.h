#pragma once

#include <vulkan/vulkan.hpp>
#include <vkbootstrap/VkBootstrap.h>
#include <Utils/Window.h>
#include <Utils/NonCopyable.h>
#include "RendererObject.h"
#include <vma/vk_mem_alloc.h>

namespace VulkanContext {
	namespace {
		vk::Instance instance;
		vk::PhysicalDevice physicalDevice;
		vk::Device device;

		VmaAllocator allocator;

		vk::DebugUtilsMessengerEXT debug_messenger; // Vulkan debug output handle

		VkSurfaceKHR surface; // Vulkan window surface		

		VkPhysicalDeviceProperties properties;

		VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugMessege(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData) {

			switch (messageSeverity)
			{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				WC_ERROR("{0}", pCallbackData->pMessage);
				//OutputDebugString(pCallbackData->pMessage);
				break;

			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				WC_WARN("{0}", pCallbackData->pMessage);
				break;

			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
				WC_INFO("{0}", pCallbackData->pMessage);
				break;

			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
				// WC_TRACE("[{0} {1} TRACE] {2}", src, typeStr, message);
				break;
			}

			return true;
		}
	}

	vk::Instance GetInstance() { return instance; }
	vk::PhysicalDevice GetPhysicalDevice() { return physicalDevice; }
	vk::Device GetDevice() { return device; }
	VkSurfaceKHR GetSurface() { return surface; }
	VmaAllocator GetMemoryAllocator() { return allocator; }
	VkPhysicalDeviceProperties GetProperties() { return properties; }

	size_t pad_uniform_buffer_size(size_t originalSize)
	{
		// Calculate required alignment based on minimum device offset alignment
		size_t minUboAlignment = properties.limits.minUniformBufferOffsetAlignment;
		size_t alignedSize = originalSize;
		if (minUboAlignment > 0) 
			alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
		
		return alignedSize;
	}	

	vkb::Device Create(const char* AppName, const wc::Window& window) {
		vkb::InstanceBuilder builder;

		//make the vulkan instance, with basic debug features
		auto inst_ret = builder.set_app_name(AppName)
			.request_validation_layers(true)
			.set_debug_callback((PFN_vkDebugUtilsMessengerCallbackEXT)VulkanDebugMessege)
			.require_api_version(1, 1, 0)
			.build();

		vkb::Instance vkb_inst = inst_ret.value();

		//grab the instance 
		instance = vkb_inst.instance;
		debug_messenger = vkb_inst.debug_messenger;

		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) WC_ERROR("Failed to create window surface!");

		VkPhysicalDeviceMultiDrawFeaturesEXT reqFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTI_DRAW_FEATURES_EXT };
		reqFeatures.multiDraw = true;
		VkPhysicalDeviceFeatures features = {};
		features.multiDrawIndirect = true;

		vkb::PhysicalDeviceSelector selector{ vkb_inst };
		vkb::PhysicalDevice physDevice = selector
			.set_minimum_version(1, 1)
			.set_surface(surface)
			.set_required_features(features)
			.select()
			.value();

		properties = physDevice.properties;

		//create the final vulkan device

		vkb::DeviceBuilder deviceBuilder{ physDevice };
		deviceBuilder.add_pNext(&reqFeatures);

		vkb::Device vkbDevice = deviceBuilder.build().value();

		// Get the VkDevice handle used in the rest of a vulkan application
		device = vkbDevice.device;
		physicalDevice = physDevice.physical_device;

		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = physicalDevice;
		allocatorInfo.device = device;
		allocatorInfo.instance = instance;
		vmaCreateAllocator(&allocatorInfo, &allocator);

		return vkbDevice;
	}	

	/*const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	const bool enableValidationLayers = true;

	bool checkValidationLayerSupport() {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}

	std::vector<const char*> getRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	void setupDebugMessenger() {
		if (!enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		createInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = VulkanDebugMessege;

		if (vkCreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debug_messenger) != VK_SUCCESS) 
			WC_ERROR("failed to set up debug messenger!");		
	}

	void createInstance(const char* AppName) {
		if (enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 1, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 1, 0);
		appInfo.apiVersion = VK_API_VERSION_1_1;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			debugCreateInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
			debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			debugCreateInfo.pfnUserCallback = VulkanDebugMessege;

			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else {
			createInfo.enabledLayerCount = 0;

			createInfo.pNext = nullptr;
		}

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}
	}*/

	void Destroy() {
		vmaDestroyAllocator(allocator);

		device.destroy();
		instance.destroySurfaceKHR(surface);
		vkb::destroy_debug_utils_messenger(instance, debug_messenger);
		instance.destroy();
	}
}