#pragma once
#include "stuff.h"

//common mostly swapchain
namespace tde {

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities = {};
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
	//---swapchain---
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t desired_width, uint32_t desired_height);
}

//InstanceBuilder declaration
namespace tde {

	struct BuildContext {
		std::string app_name;
		uint32_t api_version = VK_API_VERSION_1_0;
		VkInstance instance = VK_NULL_HANDLE;
		bool enableValidationLayers = false;
		std::vector<const char*> validationLayers;
		bool useDebugMessenger = false;
		VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;            //for debugging and validation layers
	};

	std::vector<const char*> getRequiredExtensions();
	static VKAPI_ATTR VkBool32 VKAPI_CALL defaultDebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

	//debugging support
	const std::vector<const char*> defaultValidationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};





	struct InstanceData
	{
		bool enableValidationLayers = false;
		VkInstance instance = VK_NULL_HANDLE;
		std::string app_name;
		bool useDebugMessenger = false;
		uint32_t api_version = VK_API_VERSION_1_0;
		PFN_vkDebugUtilsMessengerCallbackEXT debug_callback = defaultDebugCallback;
		VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;            //for debugging and validation layers
		std::vector<const char*> extensions;
		std::vector<const char*> validationLayers = defaultValidationLayers;
	};

	class InstanceBuilder {
	public:
		InstanceData data;
		bool enableValidationLayers = false;
		VkInstance instance = VK_NULL_HANDLE;
		std::string app_name;
		bool useDebugMessenger = false;

		//VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
		//VkAllocationCallbacks* allocation_callbacks = VK_NULL_HANDLE;
		//PFN_vkGetInstanceProcAddr fp_vkGetInstanceProcAddr = VK_NULL_HANDLE;
		//PFN_vkGetDeviceProcAddr fp_vkGetDeviceProcAddr = VK_NULL_HANDLE;
		//uint32_t instance_version = VKB_VK_API_VERSION_1_0;
		uint32_t api_version = VK_API_VERSION_1_0;
		PFN_vkDebugUtilsMessengerCallbackEXT debug_callback = defaultDebugCallback;
		VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;            //for debugging and validation layers
		std::vector<const char*> extensions;
		std::vector<const char*> validationLayers = defaultValidationLayers;

		InstanceBuilder();

		InstanceBuilder& set_app_name(const std::string& name);
		InstanceBuilder& request_validation_layers(bool useValidationLayers);
		InstanceBuilder& use_default_debug_messenger();
		InstanceBuilder& require_api_version(int,int,int);

		InstanceData build();
	};
}


//DeviceSelector declaration
namespace tde {



	struct PhysicalDeviceData {
		VkInstance instance = VK_NULL_HANDLE;// Vulkan library handle
		VkSurfaceKHR surface = VK_NULL_HANDLE;// Vulkan window surface
		uint32_t api_version = VK_API_VERSION_1_0;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;// GPU chosen as the default device
		QueueFamilyIndices queueFamilyIndices;
		SwapChainSupportDetails swapChainSupportDetails;
		std::vector<const char*> deviceExtensions;
	};

	

	class PhysicalDeviceSelector {


		void pickPhysicalDevice();
		bool isDeviceSuitable(VkPhysicalDevice device);
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		

	public:

		//again with the hardcoded sh*
		const std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		PhysicalDeviceData physicalDevice;
		PhysicalDeviceSelector(VkInstance instance, VkSurfaceKHR surface);


		PhysicalDeviceSelector& set_minimum_version(int major, int minor);
		PhysicalDeviceSelector& set_required_features_13(VkPhysicalDeviceVulkan13Features features13);
		PhysicalDeviceSelector& set_required_features_12(VkPhysicalDeviceVulkan12Features  features12);
		PhysicalDeviceSelector& set_surface(VkSurfaceKHR surface);
		PhysicalDeviceData select();
	};
}

//DeviceBuilder declaration
namespace tde {
	class DeviceBuilder {
	public:
		PhysicalDeviceData physicalDevice;// GPU chosen as the default device
		VkDevice device = VK_NULL_HANDLE;
		VkQueue graphicsQueue = VK_NULL_HANDLE;  //where we put draw commands
		VkQueue presentQueue = VK_NULL_HANDLE;
		uint32_t graphicsQueueFamily = 0;
		uint32_t presentQueueFamily = 0;
		DeviceBuilder(PhysicalDeviceData physicalDevice);
		VkDevice build();
	};
}


namespace tde {
	auto GetSupportedInstanceExtensions();

	auto GetSupportedInstanceLayers();
}

#ifdef INIT_IMPLEMENTATION
namespace tde {
	auto GetSupportedInstanceExtensions() {
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
		return extensions;
	}

	auto GetSupportedInstanceLayers() {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
		return availableLayers;
	}
}


namespace tde {
	///this is a bit to hardcoded for my taste, the InstanceBuilder should maybe have a "default" or it should be added to it 
	std::vector<const char*> getRequiredExtensions() {

		std::vector<const char*> extensions{
			VK_KHR_SURFACE_EXTENSION_NAME,
		};

		//if windows
		extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

		return extensions;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL defaultDebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		//createInfo.pfnUserCallback = defaultDebugCallback;
	}


	//extentions methods for debugging
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}
}

//Commons implementation
namespace tde {

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

			if (presentSupport) {
				indices.presentFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}

			i++;
		}

		return indices;
	}

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
		SwapChainSupportDetails details;

		//query basic surface capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		//query supported surface formats
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		//query supported presentation modes
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	//find the "optimal" settings for the swap chain
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}
		//do logic for next best thing but we just go with the first...
		return availableFormats[0];
	}


	//find best present mode, fifo always availible bet tutorial likes Mailbox so we see if that exists 
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	//the desired size/resolution
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t desired_width, uint32_t desired_height) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>().max()) {
			return capabilities.currentExtent;
		}
		else {

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(desired_width),
				static_cast<uint32_t>(desired_height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}
}

//InstanceBuilder implementation
namespace tde {

	InstanceBuilder::InstanceBuilder() {

	}

	InstanceBuilder& InstanceBuilder::set_app_name(const std::string& name) {
		app_name = name;
		return *this;
	}
	InstanceBuilder& InstanceBuilder::request_validation_layers(bool useValidationLayers) {
		enableValidationLayers = useValidationLayers;
		return *this;
	}

	InstanceBuilder& InstanceBuilder::use_default_debug_messenger() {
		useDebugMessenger = true;
		debug_callback = defaultDebugCallback;
		return *this;
	}
	InstanceBuilder& InstanceBuilder::require_api_version(int major, int minor, int patch) {
		
		api_version = VK_MAKE_API_VERSION(0, major, minor, patch);
		return *this;
	}

	InstanceData InstanceBuilder::build() {
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = app_name.c_str();
		appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
		appInfo.pEngineName = "Top Dog Engine";
		appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
		appInfo.apiVersion = api_version;
		
		//BEGIN VkInstanceCreateInfo
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		//auto avalibleExtensions = GetSupportedInstanceExtensions();
		//std::cout << "available extensions:" << std::endl;
		//for (const auto& extension : avalibleExtensions) {
		//	std::cout << '\t' << extension.extensionName << std::endl;
		//}

		//auto avalibleLayers = GetSupportedInstanceLayers();
		//std::cout << "available layers:" << std::endl;
		//for (const auto& layer : avalibleLayers) {
		//	std::cout << '\t' << layer.layerName << std::endl;
		//}



		auto extensions = getRequiredExtensions();
		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
			populateDebugMessengerCreateInfo(debugCreateInfo);
			debugCreateInfo.pfnUserCallback = debug_callback;
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else {
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}


		if (!enableValidationLayers)
		{ 
		
			VkDebugUtilsMessengerCreateInfoEXT createInfo;
			populateDebugMessengerCreateInfo(createInfo);
			createInfo.pfnUserCallback = debug_callback;
			if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
				throw std::runtime_error("failed to set up debug messenger!");
			}
		}


		return data;
	}
}


//DeviceSelector implementation
namespace tde {
	PhysicalDeviceSelector::PhysicalDeviceSelector(VkInstance instance, VkSurfaceKHR surface) {
		physicalDevice.instance = instance;
		physicalDevice.surface = surface;
	}

	PhysicalDeviceSelector& PhysicalDeviceSelector::set_minimum_version(int major, int minor) {
		physicalDevice.api_version = VK_MAKE_API_VERSION(0, major, minor, 0);
		return *this;
	}
	PhysicalDeviceSelector& PhysicalDeviceSelector::set_required_features_13(VkPhysicalDeviceVulkan13Features features13) {
		return *this;
	}
	PhysicalDeviceSelector& PhysicalDeviceSelector::set_required_features_12(VkPhysicalDeviceVulkan12Features  features12){
		return *this;
	}
	PhysicalDeviceSelector& PhysicalDeviceSelector::set_surface(VkSurfaceKHR surface){
		physicalDevice.surface = surface;
		return *this;
	}
	PhysicalDeviceData PhysicalDeviceSelector::select(){
		physicalDevice.deviceExtensions = deviceExtensions;
		pickPhysicalDevice();
		return physicalDevice;
	}

	void PhysicalDeviceSelector::pickPhysicalDevice() {
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(physicalDevice.instance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(physicalDevice.instance, &deviceCount, devices.data());

		//VkPhysicalDeviceProperties props;

		for (const auto& device : devices) {
			
			//vkGetPhysicalDeviceProperties(device, &props);
			//printl("Device found ", props.deviceName);
			if (isDeviceSuitable(device)) {
				physicalDevice.physicalDevice = device;
				break;
			}
		}

		//vkGetPhysicalDeviceProperties(physicalDevice.physicalDevice, &props);
		//printl("Device selected ", props.deviceName);

		if (physicalDevice.physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}

	}

	bool PhysicalDeviceSelector::isDeviceSuitable(VkPhysicalDevice device) {

		physicalDevice.queueFamilyIndices = findQueueFamilies(device, physicalDevice.surface);

		//make sure we have
		bool extensionsSupported = checkDeviceExtensionSupport(device);

		//query if we have a swap-chain that works, basically any swap-chain
		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, physicalDevice.surface);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
			physicalDevice.swapChainSupportDetails = swapChainSupport;
		}
		return physicalDevice.queueFamilyIndices.isComplete() && extensionsSupported && swapChainAdequate;
	}



	bool PhysicalDeviceSelector::checkDeviceExtensionSupport(VkPhysicalDevice device) {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}




}

//DeviceBuilder implementation
namespace tde {
	DeviceBuilder::DeviceBuilder(PhysicalDeviceData physicalDevice):physicalDevice(physicalDevice) {}

	VkDevice DeviceBuilder::build() {
		//QueueFamilyIndices indices = physicalDevice.queueFamilyIndices;
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice.physicalDevice, physicalDevice.surface);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
		graphicsQueueFamily = indices.graphicsFamily.value();
		presentQueueFamily = indices.presentFamily.value();


		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &deviceFeatures;


		//for (size_t i = 0; i < physicalDevice.deviceExtensions.size(); i++)
		//{
		//	printl("Device name: ", physicalDevice.deviceExtensions[i]);
		//}

		createInfo.enabledExtensionCount = static_cast<uint32_t>(physicalDevice.deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = physicalDevice.deviceExtensions.data();


//here

		VkPhysicalDeviceVulkan13Features features13 = {};
		features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
		// set the desired features here:
		features13.dynamicRendering = VK_TRUE;
		features13.synchronization2 = VK_TRUE;


		//vulkan 1.2 features
		VkPhysicalDeviceVulkan12Features features12 = {};
		features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		features12.bufferDeviceAddress = VK_TRUE;
		features12.descriptorIndexing = VK_TRUE;
		features12.pNext = &features13;

		VkPhysicalDeviceFeatures features = {};
		VkPhysicalDeviceFeatures2 features2 = {};
		features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		features2.pNext = &features12;
		features2.features = features;

		createInfo.pNext = &features2;
		createInfo.pEnabledFeatures = NULL; //sligtly odd
//to here is to enable syncronization2

		//this is from vulkan tutorial and I'm no sure how the vkbootstrapper handles this case
		//if (enableValidationLayers) {
		//	createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		//	createInfo.ppEnabledLayerNames = validationLayers.data();
		//}
		//else {
		//	createInfo.enabledLayerCount = 0;
		//}

		if (vkCreateDevice(physicalDevice.physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}

		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);

		//printl("Success on device creation and queue getting");

		return device;
	}
}


#endif