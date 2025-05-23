#pragma once
#include "stuff.h"

/*
	I dont like this file, the name is wrong, it include stuff that is barly functioning...
	Do it better!
*/

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
//#define INIT_IMPLEMENTATION
#ifdef INIT_IMPLEMENTATION



#endif