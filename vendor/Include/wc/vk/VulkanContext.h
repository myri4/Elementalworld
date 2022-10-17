#pragma once

#include "../Utils/Window.h"
#include "RendererObject.h"
#include <vma/vk_mem_alloc.h>
#include <unordered_set>

enum class Vendor : uint32_t {
	AMD = 0x1002,
	ImgTec = 0x1010,
	NVIDIA = 0x10DE,
	ARM = 0x13B5,
	Qualcomm = 0x5143,
	INTEL = 0x8086
};

const bool enableValidationLayers = false;

namespace VulkanContext {
	namespace {
		VkInstance instance; //@TOOD: make an abstraction
		VkPhysicalDevice physicalDevice; //@TOOD: make an abstraction
		VkDevice device; //@TOOD: make an abstraction

		VmaAllocator allocator; // @TODO: remove

		//@TOOD: make an abstraction
		VkDebugUtilsMessengerEXT debug_messenger; // Vulkan debug output handle	

		VkPhysicalDeviceProperties properties;

		VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
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

		PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabel = nullptr;
		PFN_vkCmdInsertDebugUtilsLabelEXT vkCmdInsertDebugUtilsLabel = nullptr;
		PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabel = nullptr;

		PFN_vkQueueBeginDebugUtilsLabelEXT vkQueueBeginDebugUtilsLabel = nullptr;
		PFN_vkQueueInsertDebugUtilsLabelEXT vkQueueInsertDebugUtilsLabel = nullptr;
		PFN_vkQueueEndDebugUtilsLabelEXT vkQueueEndDebugUtilsLabel = nullptr;

		PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectName = nullptr;
	}

	VkInstance GetInstance() { return instance; }
	VkPhysicalDevice GetPhysicalDevice() { return physicalDevice; }
	VkDevice GetDevice() { return device; }
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

	void BeginLabel(const VkCommandBuffer& command_buffer, const char* label_name, const glm::vec4& color = glm::vec4(1.f))
	{
		VkDebugUtilsLabelEXT label = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
		label.pLabelName = label_name;
		label.color[0] = color[0];
		label.color[1] = color[1];
		label.color[2] = color[2];
		label.color[3] = color[3];
		if (enableValidationLayers) vkCmdBeginDebugUtilsLabel(command_buffer, &label);
	}

	void InsertLabel(const VkCommandBuffer& command_buffer, const char* label_name, const glm::vec4& color = glm::vec4(1.f))
	{
		VkDebugUtilsLabelEXT label = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
		label.pLabelName = label_name;
		label.color[0] = color[0];
		label.color[1] = color[1];
		label.color[2] = color[2];
		label.color[3] = color[3];
		if (enableValidationLayers) vkCmdInsertDebugUtilsLabel(command_buffer, &label);
	}

	void EndLabel(VkCommandBuffer command_buffer) { if (enableValidationLayers) vkCmdEndDebugUtilsLabel(command_buffer); }


	void BeginLabel(const VkQueue& queue, const char* label_name, const glm::vec4& color = glm::vec4(1.f))
	{
		VkDebugUtilsLabelEXT label = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
		label.pLabelName = label_name;
		label.color[0] = color[0];
		label.color[1] = color[1];
		label.color[2] = color[2];
		label.color[3] = color[3];
		if (enableValidationLayers) vkQueueBeginDebugUtilsLabel(queue, &label);
	}

	void InsertLabel(const VkQueue& queue, const char* label_name, const glm::vec4& color = glm::vec4(1.f))
	{
		VkDebugUtilsLabelEXT label = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
		label.pLabelName = label_name;
		label.color[0] = color[0];
		label.color[1] = color[1];
		label.color[2] = color[2];
		label.color[3] = color[3];
		if (enableValidationLayers) vkQueueInsertDebugUtilsLabel(queue, &label);
	}

	void EndLabel(const VkQueue& queue) { vkQueueEndDebugUtilsLabel(queue); }

	void SetObjectName(const VkObjectType& object_type, const uint64_t& object_handle, const char* object_name)
	{
		VkDebugUtilsObjectNameInfoEXT name_info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
		name_info.objectType = object_type;
		name_info.objectHandle = object_handle;
		name_info.pObjectName = object_name;
		if (enableValidationLayers) vkSetDebugUtilsObjectName(device, &name_info);
	}



	const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
	const std::vector<const char*> deviceExtensions = {	VK_KHR_SWAPCHAIN_EXTENSION_NAME	};


	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
		std::optional<uint32_t> computeFamily;
		std::optional<uint32_t> transferFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	std::vector<const char*> getRequiredExtensions() { // @TODO: remove
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers) 
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);		

		return extensions;
	}

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = DebugCallback;
	}

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
		auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (vkCreateDebugUtilsMessengerEXT != nullptr) 
			return vkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pDebugMessenger);		
		else 
			return VK_ERROR_EXTENSION_NOT_PRESENT;		
	}

	void setupDebugMessenger() {
		if (!enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debug_messenger) != VK_SUCCESS) 
			WC_ERROR("Failed to set up debug messenger!");		
	}

	bool checkValidationLayerSupport() {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers) 
			for (const auto& layerProperties : availableLayers) 
				if (strcmp(layerName, layerProperties.layerName) == 0) 
					return true;	

		return false;
	}

	void createInstance() {
		// @TODO: add mac support

		if (enableValidationLayers && !checkValidationLayerSupport()) 
			WC_ERROR("Validation layers requested, but not available!");		

		VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
		appInfo.pApplicationName = "WC Application";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 1, 0);
		appInfo.pEngineName = "WC Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 1, 0);
		appInfo.apiVersion = VK_API_VERSION_1_1;

		VkInstanceCreateInfo createInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) 
			WC_ERROR("Failed to create instance!");		
	}

	bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::unordered_set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions) 
			requiredExtensions.erase(extension.extensionName);		

		return requiredExtensions.empty();
	}

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		for (const auto& availableFormat : availableFormats) 
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return availableFormat;		

		return availableFormats[0];
	}

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		for (const auto& availablePresentMode : availablePresentModes) 
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) 
				return availablePresentMode;		

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
		SwapChainSupportDetails details;

		auto& surface = wc::window.surface;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.graphicsFamily = i;
			if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) 	indices.computeFamily = i;
			if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) indices.transferFamily = i;			

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, wc::window.surface, &presentSupport);

			if (presentSupport) 
				indices.presentFamily = i;			

			if (indices.isComplete()) 
				break;			

			i++;
		}

		return indices;
	}

	bool isDeviceSuitable(VkPhysicalDevice device) {
		QueueFamilyIndices indices = findQueueFamilies(device);

		bool extensionsSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		return indices.isComplete() && extensionsSupported && swapChainAdequate;
	}

	void pickPhysicalDevice() {
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount == 0) 
			WC_ERROR("failed to find GPUs with Vulkan support!");
		

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				physicalDevice = device;
				return;
			}
		}

		if (physicalDevice == VK_NULL_HANDLE) 
			WC_ERROR("failed to find a suitable GPU!");		
	}	

	class Queue : public RendererObject<VkQueue> {
		uint32_t queueFamily; //family of that queue
	public:

		void GetDeviceQueue(const uint32_t& family) {
			queueFamily = family;
			vkGetDeviceQueue(device, family, 0, &m_RendererID);
		}

		VkResult Submit(const VkSubmitInfo& submit_info, const VkFence& fence) const { return vkQueueSubmit(m_RendererID, 1, &submit_info, fence); }

		VkResult PresentKHR(const VkPresentInfoKHR& present_info) const { return vkQueuePresentKHR(m_RendererID, &present_info); }

		void WaitIdle() { vkQueueWaitIdle(m_RendererID); }

		uint32_t GetFamily() const { return queueFamily; }
	};

	Queue graphicsQueue;
	Queue presentQueue;
	Queue computeQueue;
	Queue transferQueue;

	void createDevice() {
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::unordered_set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.multiDrawIndirect = true; // optional

		VkDeviceCreateInfo createInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };

		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else 
			createInfo.enabledLayerCount = 0;		

		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) 
			WC_INFO("failed to create logical device!");		

		graphicsQueue.GetDeviceQueue(indices.graphicsFamily.value());
		presentQueue.GetDeviceQueue(indices.presentFamily.value());
		computeQueue.GetDeviceQueue(indices.computeFamily.value());
		transferQueue.GetDeviceQueue(indices.transferFamily.value());
	}

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
			return capabilities.currentExtent;		
		else {
			int width, height;
			glfwGetFramebufferSize(wc::window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	void createSwapChain(wc::Window& window) {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) 
			imageCount = swapChainSupport.capabilities.maxImageCount;		

		VkSwapchainCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
		createInfo.surface = wc::window.surface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else 
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &window.swapchain) != VK_SUCCESS) 
			WC_ERROR("failed to create swap chain!");		

		vkGetSwapchainImagesKHR(device, window.swapchain, &imageCount, nullptr);
		window.swapchainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, window.swapchain, &imageCount, window.swapchainImages.data());

		window.swapchainImageFormat = surfaceFormat.format;

		window.swapchainImageViews.resize(window.swapchainImages.size());

		for (size_t i = 0; i < window.swapchainImages.size(); i++) {
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = window.swapchainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = window.swapchainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device, &createInfo, nullptr, &window.swapchainImageViews[i]) != VK_SUCCESS)
				WC_ERROR("Failed to create image views!");
		}
	}

	void Create(wc::Window& window) {
		createInstance();
		setupDebugMessenger();

		if (glfwCreateWindowSurface(instance, window, nullptr, &wc::window.surface) != VK_SUCCESS) WC_ERROR("failed to create window surface!");

		pickPhysicalDevice();
		createDevice();

		createSwapChain(window);

		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = physicalDevice;
		allocatorInfo.device = device;
		allocatorInfo.instance = instance;
		vmaCreateAllocator(&allocatorInfo, &allocator);

		if (enableValidationLayers) {
			vkCmdBeginDebugUtilsLabel = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT");
			vkCmdInsertDebugUtilsLabel = (PFN_vkCmdInsertDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdInsertDebugUtilsLabelEXT");
			vkCmdEndDebugUtilsLabel = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT");

			vkQueueBeginDebugUtilsLabel = (PFN_vkQueueBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkQueueBeginDebugUtilsLabelEXT");
			vkQueueInsertDebugUtilsLabel = (PFN_vkQueueInsertDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkQueueInsertDebugUtilsLabelEXT");
			vkQueueEndDebugUtilsLabel = (PFN_vkQueueEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkQueueEndDebugUtilsLabelEXT");

			vkSetDebugUtilsObjectName = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");
		}
	}

	void Destroy() {
		vkDestroySurfaceKHR(instance, wc::window.surface, nullptr);

		vmaDestroyAllocator(allocator);
		vkDestroyDevice(device, nullptr);

		auto vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (vkDestroyDebugUtilsMessengerEXT != nullptr)
			vkDestroyDebugUtilsMessengerEXT(instance, debug_messenger, nullptr);

		vkDestroyInstance(instance, nullptr);
	}
}

namespace wc {
	using Queue = VulkanContext::Queue;
}