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

#define WC_ENABLE_GRAPHICS_DEBUGGER

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
#ifdef WC_ENABLE_GRAPHICS_DEBUGGER
		VkDebugUtilsLabelEXT label = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
		label.pLabelName = label_name;
		label.color[0] = color[0];
		label.color[1] = color[1];
		label.color[2] = color[2];
		label.color[3] = color[3];
		vkCmdBeginDebugUtilsLabel(command_buffer, &label);
#endif
	}

	void InsertLabel(const VkCommandBuffer& command_buffer, const char* label_name, const glm::vec4& color = glm::vec4(1.f))
	{
#ifdef WC_ENABLE_GRAPHICS_DEBUGGER
		VkDebugUtilsLabelEXT label = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
		label.pLabelName = label_name;
		label.color[0] = color[0];
		label.color[1] = color[1];
		label.color[2] = color[2];
		label.color[3] = color[3];
		vkCmdInsertDebugUtilsLabel(command_buffer, &label);
#endif
	}

	void EndLabel(VkCommandBuffer command_buffer) { 
#ifdef WC_ENABLE_GRAPHICS_DEBUGGER
		vkCmdEndDebugUtilsLabel(command_buffer); 
#endif // WC_ENABLE_GRAPHICS_DEBUGGER
	}


	void BeginLabel(const VkQueue& queue, const char* label_name, const glm::vec4& color = glm::vec4(1.f))
	{
#ifdef WC_ENABLE_GRAPHICS_DEBUGGER
		VkDebugUtilsLabelEXT label = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
		label.pLabelName = label_name;
		label.color[0] = color[0];
		label.color[1] = color[1];
		label.color[2] = color[2];
		label.color[3] = color[3];
		vkQueueBeginDebugUtilsLabel(queue, &label);
#endif // WC_ENABLE_GRAPHICS_DEBUGGER
	}

	void InsertLabel(const VkQueue& queue, const char* label_name, const glm::vec4& color = glm::vec4(1.f))
	{
#ifdef WC_ENABLE_GRAPHICS_DEBUGGER
		VkDebugUtilsLabelEXT label = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
		label.pLabelName = label_name;
		label.color[0] = color[0];
		label.color[1] = color[1];
		label.color[2] = color[2];
		label.color[3] = color[3];
		vkQueueInsertDebugUtilsLabel(queue, &label);
#endif // WC_ENABLE_GRAPHICS_DEBUGGER
	}

	void EndLabel(const VkQueue& queue) { vkQueueEndDebugUtilsLabel(queue); }

	void SetObjectName(const VkObjectType& object_type, const uint64_t& object_handle, const char* object_name)
	{
#ifdef WC_ENABLE_GRAPHICS_DEBUGGER
		VkDebugUtilsObjectNameInfoEXT name_info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
		name_info.objectType = object_type;
		name_info.objectHandle = object_handle;
		name_info.pObjectName = object_name;
		vkSetDebugUtilsObjectName(device, &name_info);
#endif // WC_ENABLE_GRAPHICS_DEBUGGER
	}



	const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
	const std::vector<const char*> deviceExtensions = {	VK_KHR_SWAPCHAIN_EXTENSION_NAME	};


	

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		//std::optional<uint32_t> presentFamily;
		std::optional<uint32_t> computeFamily;
		std::optional<uint32_t> transferFamily;

		bool isComplete() {
			return 
				graphicsFamily.has_value() && 
				//presentFamily.has_value() &&
				computeFamily.has_value() &&
				transferFamily.has_value();
		}
	};

	std::vector<const char*> getRequiredExtensions() { // @TODO: remove
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef WC_ENABLE_GRAPHICS_DEBUGGER
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);		
#endif // WC_ENABLE_GRAPHICS_DEBUGGER

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

#ifdef WC_ENABLE_GRAPHICS_DEBUGGER
		if (!checkValidationLayerSupport()) 
			WC_ERROR("Validation layers requested, but not available!");
#endif

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
#ifdef WC_ENABLE_GRAPHICS_DEBUGGER
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
#endif

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

	

	

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device/*, VkSurfaceKHR surface*/) {
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

			//VkBool32 presentSupport = false;
			//vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			//
			//if (presentSupport) 
			//	indices.presentFamily = i;			

			if (indices.isComplete()) 
				return indices;

			i++;
		}

		return indices;
	}

	bool isDeviceSuitable(VkPhysicalDevice device/*, VkSurfaceKHR surface*/) {
		QueueFamilyIndices indices = findQueueFamilies(device/*, surface*/);

		bool extensionsSupported = checkDeviceExtensionSupport(device);

		//bool swapChainAdequate = false;
		//if (extensionsSupported) {
		//	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
		//	swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		//}

		return indices.isComplete() && extensionsSupported /*&& swapChainAdequate*/;
	}

	void pickPhysicalDevice(/*VkSurfaceKHR surface*/) {
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount == 0) 
			WC_ERROR("failed to find GPUs with Vulkan support!");
		

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		for (const auto& device : devices) {
			if (isDeviceSuitable(device/*, surface*/)) {
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
	//Queue presentQueue;
	Queue computeQueue;
	Queue transferQueue;

	void createDevice(/*VkSurfaceKHR surface*/) {
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice/*, surface*/);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::unordered_set<uint32_t> uniqueQueueFamilies = { 
			indices.graphicsFamily.value(), 
			//indices.presentFamily.value(),
			indices.computeFamily.value(),
			indices.transferFamily.value(),
		};

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
		createInfo.enabledLayerCount = 0;		

#ifdef WC_ENABLE_GRAPHICS_DEBUGGER
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
#endif

		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) 
			WC_INFO("failed to create logical device!");		

		graphicsQueue.GetDeviceQueue(indices.graphicsFamily.value());
		//presentQueue.GetDeviceQueue(indices.presentFamily.value());
		computeQueue.GetDeviceQueue(indices.computeFamily.value());
		transferQueue.GetDeviceQueue(indices.transferFamily.value());
	}

	inline void Create(wc::Window& window) {
		createInstance();
		
#ifdef WC_ENABLE_GRAPHICS_DEBUGGER
			VkDebugUtilsMessengerCreateInfoEXT createInfo;
			populateDebugMessengerCreateInfo(createInfo);

			if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debug_messenger) != VK_SUCCESS)
				WC_ERROR("Failed to set up debug messenger!");


			vkCmdBeginDebugUtilsLabel = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT");
			vkCmdInsertDebugUtilsLabel = (PFN_vkCmdInsertDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdInsertDebugUtilsLabelEXT");
			vkCmdEndDebugUtilsLabel = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT");

			vkQueueBeginDebugUtilsLabel = (PFN_vkQueueBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkQueueBeginDebugUtilsLabelEXT");
			vkQueueInsertDebugUtilsLabel = (PFN_vkQueueInsertDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkQueueInsertDebugUtilsLabelEXT");
			vkQueueEndDebugUtilsLabel = (PFN_vkQueueEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkQueueEndDebugUtilsLabelEXT");

			vkSetDebugUtilsObjectName = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");
#endif // WC_ENABLE_GRAPHICS_DEBUGGER

		pickPhysicalDevice(/*window.surface*/);
		createDevice(/*window.surface*/);

		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = physicalDevice;
		allocatorInfo.device = device;
		allocatorInfo.instance = instance;
		vmaCreateAllocator(&allocatorInfo, &allocator);		

		window.CreateSwapchain(VulkanContext::GetPhysicalDevice(), VulkanContext::GetDevice(), VulkanContext::GetInstance());
	}

	void Destroy() {
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