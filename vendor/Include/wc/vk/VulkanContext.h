#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vma/vk_mem_alloc.h>
#include <unordered_set>

#define WC_GRAPHICS_DEBUGGER 1
#define WC_SHADER_DEBUG_PRINT 0

#define VK_CHECK(x)                                                 \
	do                                                              \
	{                                                               \
		VkResult err = x;                                           \
		if (err)                                                    \
		{                                                           \
			WC_ERROR("{0} returned: {1}", #x, std::string(magic_enum::enum_name((VkResult)err))); \
			abort();                                                \
		}                                                           \
	} while (0)

template <class T>
class RendererObject {
protected:
	T m_RendererID = VK_NULL_HANDLE;
public:

	operator T& () { return m_RendererID; }
	operator const T& () const { return m_RendererID; }
	operator bool() const { return m_RendererID != VK_NULL_HANDLE; }

	T* GetPointer() { return &m_RendererID; }
	const T* GetPointer() const { return &m_RendererID; }
};

namespace wc {
	enum class Vendor : uint32_t {
		AMD = 0x1002,
		ImgTec = 0x1010,
		NVIDIA = 0x10DE,
		ARM = 0x13B5,
		Qualcomm = 0x5143,
		INTEL = 0x8086
	};

	class Instance : public RendererObject<VkInstance> {
	public:
		VkResult Create(const VkInstanceCreateInfo& createInfo) {
			return vkCreateInstance(&createInfo, nullptr, &m_RendererID);
		}

		PFN_vkVoidFunction GetProcAddress(const char* pName) {
			return vkGetInstanceProcAddr(m_RendererID, pName);
		}

		void Destroy() {
			vkDestroyInstance(m_RendererID, nullptr);
		}
	};

	class PhysicalDevice : public RendererObject<VkPhysicalDevice> {
		VkPhysicalDeviceFeatures features = {};
		VkPhysicalDeviceProperties properties = {};
		VkPhysicalDeviceMemoryProperties memoryProperties = {};
	public:
	 
		PhysicalDevice() = default;
		PhysicalDevice(const VkPhysicalDevice& device) { SetDevice(device);	}

		void SetDevice(const VkPhysicalDevice& physicalDevice) {
			m_RendererID = physicalDevice;
			vkGetPhysicalDeviceFeatures(m_RendererID, &features);
			vkGetPhysicalDeviceProperties(m_RendererID, &properties);
			vkGetPhysicalDeviceMemoryProperties(m_RendererID, &memoryProperties);
		}

		VkResult GetImageFormatProperties(const VkFormat& format, const VkImageType& type, const VkImageTiling& tiling, const VkImageUsageFlags& usage, const VkImageCreateFlags& flags, VkImageFormatProperties* pImageFormatProperties) {
			return vkGetPhysicalDeviceImageFormatProperties(m_RendererID, format, type, tiling, usage, flags, pImageFormatProperties);
		}

		void GetQueueFamilyProperties(uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties) {
			vkGetPhysicalDeviceQueueFamilyProperties(m_RendererID, pQueueFamilyPropertyCount, pQueueFamilyProperties);
		}

		VkFormatProperties GetFormatProperties(const VkFormat& format) {
			VkFormatProperties formatProperties;
			vkGetPhysicalDeviceFormatProperties(m_RendererID, format, &formatProperties);
			return formatProperties;
		}

		VkPhysicalDeviceFeatures GetFeatures() const { return features; }

		VkPhysicalDeviceProperties GetProperties() const { return properties; }

		VkPhysicalDeviceMemoryProperties GetMemoryProperties() const { return memoryProperties; }
	};
	

	class Device : public RendererObject<VkDevice> {
	public:
		VkResult Create(const VkPhysicalDevice& physicalDevice, const VkDeviceCreateInfo& createInfo) {
			return vkCreateDevice(physicalDevice, &createInfo, nullptr, &m_RendererID);
		}

		PFN_vkVoidFunction GetProcAddress(const char* pName) {
			return vkGetDeviceProcAddr(m_RendererID, pName);
		}

		void WaitIdle() {
			vkDeviceWaitIdle(m_RendererID);
		}

		void Destroy() {
			vkDestroyDevice(m_RendererID, nullptr);
		}
	};	
}

namespace VulkanContext {
	namespace {
		wc::Instance instance;
		wc::PhysicalDevice physicalDevice;
		wc::Device device;

		VmaAllocator vmaAllocator;		

#if WC_GRAPHICS_DEBUGGER
		VkDebugUtilsMessengerEXT debug_messenger; // Vulkan debug output handle	
		const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

		VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData) {

			switch (messageSeverity)
			{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				WC_ERROR(pCallbackData->pMessage);
				//__debugbreak();
				//OutputDebugString(pCallbackData->pMessage);
				break;

			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				WC_WARN(pCallbackData->pMessage);
				break;

			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
				WC_TRACE(pCallbackData->pMessage);
				break;

			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
				//WC_TRACE(pCallbackData->pMessage);
				break;
			}

			switch (messageType)
			{
			case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
				WC_WARN("Performance: {0}", pCallbackData->pMessage);
				//OutputDebugString(pCallbackData->pMessage);
				break;

			case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
				//WC_TRACE("General: {0}", pCallbackData->pMessage);
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

		PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = nullptr;
		PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = nullptr;

		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
			createInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
			createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

#if WC_SHADER_DEBUG_PRINT
			createInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
#endif

			createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			createInfo.pfnUserCallback = DebugCallback;
		}
#endif
	}

	

	wc::Instance& GetInstance() { return instance; }
	wc::PhysicalDevice& GetPhysicalDevice() { return physicalDevice; }
	wc::Device& GetDevice() { return device; }
	VmaAllocator& GetMemoryAllocator() { return vmaAllocator; }
	VkAllocationCallbacks* GetAllocator() { return nullptr; }
	VkPhysicalDeviceProperties GetProperties() { return physicalDevice.GetProperties(); }
	VkPhysicalDeviceFeatures GetSupportedFeatures() { return physicalDevice.GetFeatures(); }

	void BeginLabel(const VkCommandBuffer& command_buffer, const char* label_name, const glm::vec4& color = glm::vec4(1.f))
	{
#if WC_GRAPHICS_DEBUGGER
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
#if WC_GRAPHICS_DEBUGGER
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
#if WC_GRAPHICS_DEBUGGER
		vkCmdEndDebugUtilsLabel(command_buffer); 
#endif
	}


	void BeginLabel(const VkQueue& queue, const char* label_name, const glm::vec4& color = glm::vec4(1.f))
	{
#if WC_GRAPHICS_DEBUGGER
		VkDebugUtilsLabelEXT label = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
		label.pLabelName = label_name;
		label.color[0] = color[0];
		label.color[1] = color[1];
		label.color[2] = color[2];
		label.color[3] = color[3];
		vkQueueBeginDebugUtilsLabel(queue, &label);
#endif
	}

	void InsertLabel(const VkQueue& queue, const char* label_name, const glm::vec4& color = glm::vec4(1.f))
	{
#if WC_GRAPHICS_DEBUGGER
		VkDebugUtilsLabelEXT label = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
		label.pLabelName = label_name;
		label.color[0] = color[0];
		label.color[1] = color[1];
		label.color[2] = color[2];
		label.color[3] = color[3];
		vkQueueInsertDebugUtilsLabel(queue, &label);
#endif
	}

	void EndLabel(const VkQueue& queue) { 
#if WC_GRAPHICS_DEBUGGER
		vkQueueEndDebugUtilsLabel(queue); 
#endif
	}

	void SetObjectName(const VkObjectType& object_type, const uint64_t& object_handle, const char* object_name)
	{
#if WC_GRAPHICS_DEBUGGER
		VkDebugUtilsObjectNameInfoEXT name_info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
		name_info.objectType = object_type;
		name_info.objectHandle = object_handle;
		name_info.pObjectName = object_name;
		vkSetDebugUtilsObjectName(device, &name_info);
#endif
	}


	

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

#if WC_GRAPHICS_DEBUGGER
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);		
#endif

		return extensions;
	}
	

	QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice& device/*, VkSurfaceKHR surface*/) {
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

			if (indices.isComplete()) 
				return indices;

			i++;
		}

		return indices;
	}

	bool isDeviceSuitable(const VkPhysicalDevice& device, const std::vector<const char*>& deviceExtensions/*, VkSurfaceKHR surface*/) {
		QueueFamilyIndices indices = findQueueFamilies(device/*, surface*/);

		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::unordered_set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions)
			requiredExtensions.erase(extension.extensionName);

		bool extensionsSupported = requiredExtensions.empty();

		//bool swapChainAdequate = false;
		//if (extensionsSupported) {
		//	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
		//	swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		//}

		return indices.isComplete() && extensionsSupported /*&& swapChainAdequate*/;
	}

	class Queue : public RendererObject<VkQueue> {
		uint32_t queueFamily = 0; //family of that queue
	public:

		void GetDeviceQueue(uint32_t family) {
			queueFamily = family;
			vkGetDeviceQueue(device, family, 0, &m_RendererID);
		}

		VkResult Submit(const VkSubmitInfo& submit_info, const VkFence& fence = VK_NULL_HANDLE) const { return vkQueueSubmit(m_RendererID, 1, &submit_info, fence); }

		VkResult PresentKHR(const VkPresentInfoKHR& present_info) const { return vkQueuePresentKHR(m_RendererID, &present_info); }

		void WaitIdle() { vkQueueWaitIdle(m_RendererID); }

		uint32_t GetFamily() const { return queueFamily; }
	};

	Queue graphicsQueue;
	//Queue presentQueue;
	Queue computeQueue;
	Queue transferQueue;

	inline void Create() {
		{// Create Instance
			// @TODO: add mac support

#if WC_GRAPHICS_DEBUGGER
			uint32_t layerCount = 0;
			vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

			std::vector<VkLayerProperties> availableLayers(layerCount);
			vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

			bool bValidationLayers = false;
			for (const char* layerName : validationLayers)
				for (const auto& layerProperties : availableLayers)
					if (strcmp(layerName, layerProperties.layerName) == 0)
					{
						bValidationLayers = true;
						break;
					}

			if (!bValidationLayers)
				WC_ERROR("Validation layers requested, but not available!");
#endif

			VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
			appInfo.pApplicationName = "WC Application";
			appInfo.applicationVersion = VK_MAKE_VERSION(1, 2, 0);
			appInfo.pEngineName = "WC Engine";
			appInfo.engineVersion = VK_MAKE_VERSION(1, 2, 0);
			appInfo.apiVersion = VK_API_VERSION_1_2;

			VkInstanceCreateInfo createInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
			createInfo.pApplicationInfo = &appInfo;

			auto extensions = getRequiredExtensions();
			createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
			createInfo.ppEnabledExtensionNames = extensions.data();

#if WC_GRAPHICS_DEBUGGER
			VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);

			VkValidationFeaturesEXT validationFeatures = { VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT };
			VkValidationFeatureEnableEXT enabledFeatures[] = { 
				VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT,
				//VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT,
				//VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT
			};
			validationFeatures.pEnabledValidationFeatures = enabledFeatures;
			validationFeatures.enabledValidationFeatureCount = std::size(enabledFeatures);

			debugCreateInfo.pNext = (VkValidationFeaturesEXT*)&validationFeatures;

			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;


#endif

			if (instance.Create(createInfo) != VK_SUCCESS)
				WC_ERROR("Failed to create instance!");
		}
		
#if WC_GRAPHICS_DEBUGGER
			VkDebugUtilsMessengerCreateInfoEXT createInfo;
			populateDebugMessengerCreateInfo(createInfo);

			vkCmdBeginDebugUtilsLabel = (PFN_vkCmdBeginDebugUtilsLabelEXT)instance.GetProcAddress("vkCmdBeginDebugUtilsLabelEXT");
			vkCmdInsertDebugUtilsLabel = (PFN_vkCmdInsertDebugUtilsLabelEXT)instance.GetProcAddress("vkCmdInsertDebugUtilsLabelEXT");
			vkCmdEndDebugUtilsLabel = (PFN_vkCmdEndDebugUtilsLabelEXT)instance.GetProcAddress("vkCmdEndDebugUtilsLabelEXT");

			vkQueueBeginDebugUtilsLabel = (PFN_vkQueueBeginDebugUtilsLabelEXT)instance.GetProcAddress("vkQueueBeginDebugUtilsLabelEXT");
			vkQueueInsertDebugUtilsLabel = (PFN_vkQueueInsertDebugUtilsLabelEXT)instance.GetProcAddress("vkQueueInsertDebugUtilsLabelEXT");
			vkQueueEndDebugUtilsLabel = (PFN_vkQueueEndDebugUtilsLabelEXT)instance.GetProcAddress("vkQueueEndDebugUtilsLabelEXT");

			vkSetDebugUtilsObjectName = (PFN_vkSetDebugUtilsObjectNameEXT)instance.GetProcAddress("vkSetDebugUtilsObjectNameEXT");

			vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)instance.GetProcAddress("vkCreateDebugUtilsMessengerEXT");
			vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)instance.GetProcAddress("vkDestroyDebugUtilsMessengerEXT");

			if (vkCreateDebugUtilsMessengerEXT(instance, &createInfo, VulkanContext::GetAllocator(), &debug_messenger) != VK_SUCCESS)
				WC_ERROR("Failed to set up debug messenger!");
#endif

			const std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#if WC_GRAPHICS_DEBUGGER

#if WC_SHADER_DEBUG_PRINT
			VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME
#endif

#endif
			};

		{ // Pick physical device 
			uint32_t deviceCount = 0;
			vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

			if (deviceCount == 0)
				WC_ERROR("failed to find GPUs with Vulkan support!");


			std::vector<VkPhysicalDevice> devices(deviceCount);
			vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

			for (const auto& device : devices) {
				if (isDeviceSuitable(device, deviceExtensions)) {
					physicalDevice = device;
					break;
				}
			}

			if (physicalDevice == VK_NULL_HANDLE)
				WC_ERROR("failed to find a suitable GPU!");
		}
		{ // Create Logical Device
			QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

			std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
			std::unordered_set<uint32_t> uniqueQueueFamilies = {
				indices.graphicsFamily.value(),
				//indices.presentFamily.value(),
				indices.computeFamily.value(),
				indices.transferFamily.value(),
			};

			float queuePriorities[] = { 1.0f };
			for (uint32_t queueFamily : uniqueQueueFamilies) {
				VkDeviceQueueCreateInfo queueCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
				queueCreateInfo.queueFamilyIndex = queueFamily;
				queueCreateInfo.queueCount = (uint32_t)std::size(queuePriorities);
				queueCreateInfo.pQueuePriorities = queuePriorities;

				queueCreateInfos.push_back(queueCreateInfo);
			}

			VkPhysicalDeviceFeatures deviceFeatures{};
			if (physicalDevice.GetFeatures().samplerAnisotropy) deviceFeatures.samplerAnisotropy = true;

			VkPhysicalDeviceVulkan12Features features12 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
			features12.storageBuffer8BitAccess = true;
			features12.uniformAndStorageBuffer8BitAccess = true;
			features12.shaderInt8 = true;
			features12.scalarBlockLayout = true;

			features12.shaderSampledImageArrayNonUniformIndexing = true;
			features12.runtimeDescriptorArray = true;
			features12.descriptorBindingVariableDescriptorCount = true;
			features12.descriptorBindingPartiallyBound = true;

			VkDeviceCreateInfo createInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };

			createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
			createInfo.pQueueCreateInfos = queueCreateInfos.data();

			createInfo.pEnabledFeatures = &deviceFeatures;

			createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
			createInfo.ppEnabledExtensionNames = deviceExtensions.data();

			createInfo.pNext = &features12;

#if WC_GRAPHICS_DEBUGGER
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
#endif

			if (device.Create(physicalDevice, createInfo) != VK_SUCCESS)
				WC_ERROR("Failed to create logical device!");

			graphicsQueue.GetDeviceQueue(indices.graphicsFamily.value());
			//presentQueue.GetDeviceQueue(indices.presentFamily.value());
			computeQueue.GetDeviceQueue(indices.computeFamily.value());
			transferQueue.GetDeviceQueue(indices.transferFamily.value());
		}

		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = physicalDevice;
		allocatorInfo.device = device;
		allocatorInfo.instance = instance;
		vmaCreateAllocator(&allocatorInfo, &vmaAllocator);
	}

	void Destroy() {
		vmaDestroyAllocator(vmaAllocator);
		device.Destroy();

#if WC_GRAPHICS_DEBUGGER
		vkDestroyDebugUtilsMessengerEXT(instance, debug_messenger, VulkanContext::GetAllocator());
#endif
		instance.Destroy();
	}
}

namespace wc {
	using Queue = VulkanContext::Queue;
}