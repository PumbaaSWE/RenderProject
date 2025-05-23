#pragma once
/*
* the big header
*
* To include it in multiple files create a tdeTopDogEngine.cpp file that only has the following:

	#define TOP_DOG_IMPLEMENTATION
	#include "tdeTopDogEngine.h"

* Nothing else!
* Then you can include tdeTopDogEngine.h anywhere without duplicating
* It also improves compile times as you do not recompile the whole engine each time...
*
* =====HOW TO SET UP!=====
*
* Make Sure to have Vulkan SDK installed (https://www.lunarg.com/vulkan-sdk/)
*
* Make sure the Enviroment variable VULKAN_SDK is set to the installation folder
* probably done when installing the SDK
* in cmd: echo %VULKAN_SDK%
* result should be something like: C:\VulkanSDK\1.3.211.0
* if not set it in cmd: setx VULKAN_SDK "C:\VulkanSDK\1.3.211.0"
* in PowerShell:
* [Environment]::SetEnvironmentVariable('VULKAN_SDK','C:\VulkanSDK\1.3.211.0')
* [Environment]::GetEnvironmentVariable('VULKAN_SDK')
*
* Set up Visual Studio: (Right click Your Project under Solution in Solution Explorer>Properties)
*
* Properties>General>C++ Language Standard
* Select ISO C++20
*
* Properties>Linker>All Options>Additional Dependencies
* Add vulkan-1.lib
*
* Properties>Linker>All Options>Additional Library Directories
* Add %VULKAN_SDK%\Lib
*
* Properties>C/C++>All Options>Additional Include Directories
* Add %VULKAN_SDK%\Include
*
*/

//Example main.cpp
/*
#define TOP_DOG_IMPLEMENTATION
#include "tdeTopDogEngine.h"


class Application1 : public tde::Application
{

	float time = 0;
	int secs = 0;
	bool showTime = false;

	tde::Model plane;


public:
	Application1()
	{
		appName = "Top Dog";
	}

	void Init() override {
		plane = tde::Model(&GetRenderer(), tde::Model::cube_verts, tde::Model::cube_indices);
	}


	void FixedUpdate(float dt) override {

	}


	void Update(float dt) override {


	}
	void Render(float dt, float extrapolation) override {


		mat4_t proj = glm::perspective(glm::radians(60.0f), 720.0f / 420.0f, 0.1f, 1000.0f); //this only change when fov or zNear/zFar changes
		proj[1][1] *= -1; //glm is flipped (OpenGL v Vulkan up? y neg up or down?)

		glm::mat4 view = glm::mat4(1.0f);
		view = glm::translate(view, { 0, 0, -10 });


		glm::mat4 identity = glm::mat4(1.0f); //this is currently not used
		renderer->SetUniformBuffer(identity, view, proj);



		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, 60.0f, { 0,1,1 }); // rotate around the y axis
		plane.Draw(model);

		glm::mat4 model2 = glm::mat4(1.0f);
		model2 = glm::translate(model2, { .6, 0, 8 });
		model2 = glm::rotate(model2, 3.0f, { 0,1,0 }); // rotate around the y axis
		plane.Draw(model2);

		glm::mat4 model3 = glm::mat4(1.0f);
		model3 = glm::translate(model3, { 0, .7, 4 });
		model3 = glm::rotate(model3, 70.0f, { 1,1,0 }); // rotate around the y axis
		plane.Draw(model3);
	}

};





int main()
{
	Application1 app;
	if (app.Create(720, 420) == tde::Success)
		app.Start();
	return 0;
}
*/

/*
	Authors
	Jack Forsberg
	Johannes Widén

	Copyright (c) 2025 Team Alpha Top Dog Ace Squad
	All rights reserved.

	*--------------------*
	|                    |
	|     A cool box     |
	|                    |
	*--------------------*
*/


/*
	Change-log
	
	Version 0.0
	2025-05-19
	- First ever test of the software as one header with Vulkan 1.3
	- Read obj files, limitations: needs to be triangulated, contain no submeshes, less than 16000 verts, possibli some more
	- One hardcoded pipeline
	- Two hardecoded default shaders (vert and frag) in binary form
	- Support to read shaders in SPRV format
	- One default cube to render
	- Windows support
	
	Version 0.1
	2025-05-20
	- Fixed bug with precompiled shaders were defined in the header
	- Added authors and copyright

	Version 0.2
	2025-05-23
	- Added more KeyCodes
	- Added Known bugs to this 
*/

/*
	KNOWN BUGS:
	- Minimizing is not handled properly and validation layers freak out about it 
*/ 
#pragma once
#ifndef stuff
#define stuff

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <optional>
#include <set>
#include <algorithm>
#include <functional>
#include <deque>
#include <cassert>
#include <span>
#include <array>



#define VK_USE_PLATFORM_WIN32_KHR //if windows
#define DISABLE_VULKAN_OBS_CAPTURE = 1

#include <vulkan/vulkan.h>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#ifdef _DEBUG
#define DEBUGPRINTL(x)																											\
{																																\
	std::cout << "line: " << __LINE__ << " file: " << __FILE__ << " func: " << __func__ << "msg: " << x << std::endl;			\
}																																\																										
#else
#define DEBUGPRINTL(x) ((void)0);
#endif


#define VK_CHECK(x)                                                     \
    do {                                                                \
        VkResult err = x;                                               \
        if (err) {                                                      \
             std::cout << "line: " << __LINE__ << " file: " << __FILE__ << " func: " << __func__ << "msg: " << err << std::endl;\
            abort();                                                    \
        }                                                               \
    } while (0)

#define USING_GLM

#ifdef USING_GLM
#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using mat4_t = glm::mat4;
using vec3_t = glm::vec3;
using vec2_t = glm::vec2;
#endif // USING_GLM

template<class ...Args>
void printl(Args&&... args) {
	(std::cout << ... << args) << '\n';
}

namespace tde {
	enum TdeResult { Success, Fail };

}


struct DeletionQueue
{
	std::deque<std::function<void()>> deletors;

	void push_function(std::function<void()>&& function) {
		deletors.push_back(function);
	}

	void flush() {
		// reverse iterate the deletion queue to execute all the functions
		for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
			(*it)(); //call functors
		}

		deletors.clear();
	}
};


//REMOVE! this is so that stuff deas not get grayed out during development!!
//#define TDE_IMPLEMENTATION



#endif 
#pragma once

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
#pragma once


#ifndef vk_init
#define vk_init

//command pool
namespace vkinit {
	VkCommandPoolCreateInfo command_pool_create_info(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);
	VkCommandBufferAllocateInfo command_buffer_allocate_info(VkCommandPool pool, uint32_t count = 1);
}

// sync objects
namespace vkinit {
	VkFenceCreateInfo fence_create_info(VkFenceCreateFlags flags = 0);
	VkSemaphoreCreateInfo semaphore_create_info(VkSemaphoreCreateFlags flags = 0);
}


//cmd buffer+submit+image
namespace vkinit {
	VkCommandBufferBeginInfo command_buffer_begin_info(VkCommandBufferUsageFlags flags = 0);

	VkSemaphoreSubmitInfo semaphore_submit_info(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);
	VkCommandBufferSubmitInfo command_buffer_submit_info(VkCommandBuffer cmd);
	VkSubmitInfo2 submit_info(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo);


	VkImageSubresourceRange image_subresource_range(VkImageAspectFlags aspectMask);
}

//vk init
namespace vkinit {

	//image stuff
	VkImageCreateInfo image_create_info(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
	VkImageViewCreateInfo imageview_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);

	//TODO...

	struct VertexInputDescription {

		std::vector<VkVertexInputBindingDescription> bindings;
		std::vector<VkVertexInputAttributeDescription> attributes;

		VkPipelineVertexInputStateCreateFlags flags = 0;

	};
	//pipeline stuff
	VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_info(VkShaderStageFlagBits stage, VkShaderModule shaderModule);
	VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info();
	VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info(VkPrimitiveTopology topology);
	VkPipelineRasterizationStateCreateInfo rasterization_state_create_info(VkPolygonMode polygonMode, float lineWidth = 1.0f, VkCullModeFlags cullMode = VK_CULL_MODE_FRONT_BIT, VkFrontFace frontFace = VK_FRONT_FACE_CLOCKWISE);
	VkPipelineMultisampleStateCreateInfo multisampling_state_create_info();
	VkPipelineColorBlendAttachmentState color_blend_attachment_state();
	VkPipelineLayoutCreateInfo pipeline_layout_create_info();
	VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info(bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp);
	VkRenderingAttachmentInfo attachment_info(VkImageView view, VkClearValue* clear, VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkRenderingAttachmentInfo depth_attachment_info(VkImageView view, VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkRenderingInfo rendering_info(VkExtent2D renderExtent, VkRenderingAttachmentInfo* colorAttachment, VkRenderingAttachmentInfo* depthAttachment);
}





#endif // !vk_init 
#pragma once
#include <span>

namespace vkutil
{
    void transition_image(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
    VkFormat FindSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

}

namespace vkutil 
{
    std::vector<char> read_file(const char* filePath);
    bool load_shader_module(const std::span<char> data, VkDevice device, VkShaderModule* outShaderModule);
}
 
#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <glm/glm.hpp>

namespace obj_loader {

	using vec3_t = glm::vec3;
	using vec2_t = glm::vec2;

	struct Vertex
	{
		vec3_t pos;
		vec3_t normal;
		vec2_t uv;
	};

	struct Index
	{
		uint16_t v;
		uint16_t vn;
		uint16_t vu;
	};

static bool LoadFromFile(const std::string& fileName, std::vector<Vertex>& verticies, std::vector<uint16_t>& indicies) {

	std::ifstream f(fileName);
	if (!f.is_open()) return false;


	std::vector<vec3_t> pos;
	std::vector<vec3_t> normals;
	//std::vector<vec2_t> uvs;
	//std::vector<uint16_t> tris;
	verticies.clear();
	indicies.clear();

	std::string line;
	std::string idx_pos, idx_norm, idx_uv;
	while (std::getline(f, line))
	{
		std::stringstream s;

		s << line;

		std::string begin;
		s >> begin;

		vec3_t v;

		//starts with v_BLANK_ ...bugs... vn starts with v too -.- dont add verts on that!!!
		if (line.starts_with("v ")) {
			s >> v.x >> v.y >> v.z;
			pos.push_back(v);
			Vertex a;
			a.pos = v;
			a.normal = { 1,1,1 };
			verticies.push_back(a);
		}
		else if (line.starts_with("f")) //face i.e. triangle index
		{
			//this is a pain! f can be v1 OR v1/v2 OR v1/v2/v3 OR v1//v3 depending... 1 is index, 2 is texture index, 3 is normal index
			//as of now all things needs to be turned off when exporting to get the first version!!!
			//https://en.wikipedia.org/wiki/Wavefront_.obj_file
			//https://stackoverflow.com/questions/52824956/how-can-i-parse-a-simple-obj-file-into-triangles
			std::vector<Index> indexes;
			std::string index_string;
			//PRINT("line string = " << line);

			while (s >> index_string) {
				//PRINTL("Index string = " << index_string);
				std::istringstream index_stream(index_string);
				std::getline(index_stream, idx_pos, '/');
				std::getline(index_stream, idx_uv, '/');
				std::getline(index_stream, idx_norm, '/');
				//PRINTL("i-v = " << idx_pos << ", i-vn = " << idx_norm << ", i-vu = " << idx_uv);
				uint16_t v = std::atoi(idx_pos.c_str());
				uint16_t vn = std::atoi(idx_norm.c_str());
				uint16_t vu = std::atoi(idx_uv.c_str());

				indexes.push_back({ v,vn,vu });

				//PRINTL("v = " << v << ", vn = " << vn << ", vu = " << vu);


				indicies.push_back(v - 1);




			}

			//triangulation
			for (size_t i = 1; i + 1 < indexes.size(); i++)
			{
				const Index* p[3] = { &indexes[0], &indexes[i], &indexes[i + 1] };

				// http://www.opengl.org/wiki/Calculating_a_Surface_Normal
				glm::vec3 U(pos[(p[1]->v) - 1] - pos[(p[0]->v) - 1]);
				glm::vec3 V(pos[(p[2]->v) - 1] - pos[(p[0]->v) - 1]);
				glm::vec3 faceNormal = glm::normalize(glm::cross(U, V));

				for (size_t j = 0; j < 3; ++j)
				{

					Vertex& temp = verticies[(p[j]->v) - 1];
					//temp.uv = glm::vec2(uvs[(p[j]->vt) - 1]); //if not 0
					int normal_idx = (p[j]->vn) - 1;
					//PRINTL("normal_idx= " << normal_idx);

					auto n = faceNormal;
					if (normal_idx >= 0) {
						//PRINTL("normals size= " << normals.size());
						//PRINTL("normal_idx= " << normal_idx);
						n = glm::normalize(normals[normal_idx]);
					}

					temp.normal = n;
					//temp.normal = (normal_idx >= 0 ? normals[normal_idx] : faceNormal);
				}
			}

			//int f[3];
			//s >> f[0] >> f[1] >> f[2];
			//indicies.push_back(f[0] - 1);
			//indicies.push_back(f[1] - 1);
			//indicies.push_back(f[2] - 1);
		}
		else if (line.starts_with("vn")) //vertex normals
		{
			s >> v.x >> v.y >> v.z;
			normals.push_back(v);
		}
		else if (line.starts_with("vt")) //vertex texture coords (assumed 2d!)
		{
			//s >> uv.x >> uv.y;
			//uvs.push_back(v);
		}
	}

	//size_t len = pos.size();
	//std::vector<Vertex> verts(len);

	//bool hasNormals = len == normals.size();
	//vec3_t n{ 1,1,1 };


	//verticies.resize(len);

	//for (size_t i = 0; i < len; i++)
	//{
	//	verticies[i] = { pos[i], hasNormals ? normals[i] : n };
	//}

	return true;
}
} 


//Pipeline
namespace tde
{
	struct Pipeline {
		VkPipeline pipeline;
		VkPipelineLayout layout;



		void Destroy(VkDevice device) {
			vkDestroyPipelineLayout(device, layout, nullptr);
			vkDestroyPipeline(device, pipeline, nullptr);
		}
	};

}

//PipelineLayoutBuilder
namespace tde
{
	struct PipelineLayoutBuilder {
		std::vector<VkPushConstantRange> pushConstants{};
		std::vector<VkDescriptorSetLayout> descriptorLayouts{};

		PipelineLayoutBuilder& add_push_constants(const VkPushConstantRange& range);

		PipelineLayoutBuilder& add_descriptor_set_layout(const VkDescriptorSetLayout& descriptor_set_layout);

		VkPipelineLayout build(const VkDevice device);



	};
}

//PipelineBuilder
namespace tde {
	class PipelineBuilder {
	public:

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		VkPipelineVertexInputStateCreateInfo vertexInputInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssembly;
		VkViewport viewport;
		VkRect2D scissor;
		VkPipelineRasterizationStateCreateInfo rasterizer;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineMultisampleStateCreateInfo multisampling;
		VkPipelineDepthStencilStateCreateInfo depthStencil;
		VkPipelineLayout pipelineLayout;
		VkPipelineRenderingCreateInfo renderInfo;
		VkFormat colorAttachmentFormat;
		vkinit::VertexInputDescription vertexInputDescription;

		PipelineBuilder();// { clear(); }

		VkPipeline build_pipeline(VkDevice device);

		void clear();

		PipelineBuilder& set_pipeline_layout(VkPipelineLayout layout);
		PipelineBuilder& set_shaders(VkShaderModule vertexShader, VkShaderModule fragmentShader);
		PipelineBuilder& set_polygon_mode(VkPolygonMode mode, float lineWidth = 1.0f);
		PipelineBuilder& set_input_topology(VkPrimitiveTopology topology);
		PipelineBuilder& set_cull_mode(VkCullModeFlags cullMode, VkFrontFace frontFace);
		PipelineBuilder& set_color_attachment_format(VkFormat format);
		PipelineBuilder& set_depth_format(VkFormat format);
		PipelineBuilder& disable_depthtest();
		PipelineBuilder& enable_depthtest(bool depthWriteEnable, VkCompareOp op = VK_COMPARE_OP_GREATER_OR_EQUAL);
		PipelineBuilder& disable_blending();
		PipelineBuilder& set_multisampling_none();
		PipelineBuilder& enable_blending_additive();
		PipelineBuilder& enable_blending_alphablend();
		PipelineBuilder& set_vertex_description(vkinit::VertexInputDescription description);

	};

} 
#pragma once

namespace tde {
	class Swapchain
	{
	public:
		VkFormat colorFormat{};
		VkColorSpaceKHR colorSpace{};
		VkSwapchainKHR swapchain{ VK_NULL_HANDLE };
		std::vector<VkImage> images{};
		std::vector<VkImageView> imageViews{};
		uint32_t queueNodeIndex{ UINT32_MAX };
		VkExtent2D extent = {};

		Swapchain();
		~Swapchain();

		void Destroy();
		void SetContext(VkDevice device, VkPhysicalDevice physicalDevice, VkInstance instance, VkSurfaceKHR surface);
		void Create(int width, int height, bool vsync = false);
		VkResult AcquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t& imageIndex);
		VkResult QueuePresent(VkQueue queue, uint32_t& imageIndex, VkSemaphore waitSemaphore);

	private:
		VkInstance instance = VK_NULL_HANDLE;
		VkDevice device = VK_NULL_HANDLE;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkSurfaceKHR surface = VK_NULL_HANDLE;
	};
}
 
#ifndef RENDERER
#define RENDERER



namespace tde{



    struct Vertex {
        vec3_t pos;
        vec3_t normal;

        static vkinit::VertexInputDescription GetVertexInputDescription();
    };

    struct UniformBufferObject {
        mat4_t model;
        mat4_t view;
        mat4_t proj;
    };


	constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    struct FrameData {

        //VkCommandPool commandPool; //why is this here??
        VkCommandBuffer mainCommandBuffer;
        VkSemaphore imageAvailableSemaphore;
        VkSemaphore renderFinishedSemaphore;
        VkFence inFlightFence;
    };




	class Renderer
	{
#ifdef NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif



	public:
		int width = 0;
		int height = 0;
		VkInstance instance = VK_NULL_HANDLE;// Vulkan library handle
		VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;// Vulkan debug output handle
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;// GPU chosen as the default device
		VkDevice device = VK_NULL_HANDLE; // Vulkan device for commands
		VkSurfaceKHR surface = VK_NULL_HANDLE;// Vulkan window surface

        VkQueue graphicsQueue = VK_NULL_HANDLE;  //where we put draw commands
        uint32_t graphicsQueueFamily;

        VkQueue presentQueue = VK_NULL_HANDLE;

        VkExtent2D swapchainExtent{};
        uint32_t swapchainImageIndex;
        //VkSwapchainKHR swapchain = VK_NULL_HANDLE;

        //std::vector<VkImage> swapchainImages;
        //VkFormat swapChainImageFormat = {};
        //VkExtent2D swapChainExtent = {};

        //Device* pDevice = nullptr;

        //std::vector<VkImageView> swapchainImageViews;
        Swapchain swapchain{};




        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline graphicsPipeline = VK_NULL_HANDLE;

      //  std::vector<VkFramebuffer> swapChainFramebuffers;
        VkCommandPool commandPool = VK_NULL_HANDLE;

        VkFence imFence;
        VkCommandBuffer imCommandBuffer;


        VkRenderPass renderPass = VK_NULL_HANDLE;

        //Unifor Buffer Objects, currently not in use
        std::vector<VkBuffer> uniformBuffers{};
        std::vector<VkDeviceMemory> uniformBuffersMemory{};
        std::vector<void*> uniformBuffersMapped{};


        //DescriptorAllocator globalDescriptorAllocator;
        VkDescriptorSetLayout descriptorSetLayout{};

        //VkDescriptorSet _drawImageDescriptors;
        //VkDescriptorSetLayout _drawImageDescriptorLayout;

        VkDescriptorPool descriptorPool{};
        std::vector<VkDescriptorSet> descriptorSets{};

        std::vector<VkCommandBuffer> commandBuffers{};

        //sync objects
        //std::vector<VkSemaphore> imageAvailableSemaphores;
        //std::vector<VkSemaphore> renderFinishedSemaphores;
        //std::vector<VkFence> inFlightFences;

        //depth objects
        VkImage depthImage{};
        VkDeviceMemory depthImageMemory{};
        VkImageView depthImageView{};

        int currentFrame = 0;
        bool framebufferResized = false;
        int frameNumber = 0;
        FrameData frames[MAX_FRAMES_IN_FLIGHT];

        FrameData& get_current_frame() { return frames[frameNumber % MAX_FRAMES_IN_FLIGHT]; };

        VkPipelineLayout trianglePipelineLayout{};
        VkPipeline trianglePipeline{};

        DeletionQueue mainDeletionQueue{};


		Renderer();
		~Renderer();

		void Create(std::vector<void*> args, int width, int height);

        void BeginFrame();
        void EndFrame();
        void SetUniformBuffer(mat4_t a, mat4_t view, mat4_t proj);
        //void SetUniformBuffer(const void* pData, int size);
        //void SetPushConstant(const void* pData, int size);
        void WaitIdle();
        void SetViewport(int width, int height);

        VkCommandBuffer& GetCommandBuffer();

	private:
        bool resize_requested = false;
		void Init(int width, int height);
		void CreateSurfaceOnWindows(HWND hwnd, HINSTANCE hInstance);
        void CreateSwapchain(uint32_t width, uint32_t height);
        void ResizeSwapchain();
        void DestroySwapchain();
		void InitCommands();
		void InitSyncStructures();
        void CreateUniformBuffers();
		void InitDescriptors();
        void InitPipelines();
        void InitDefaultPipeline();
		void Destroy();



    public:

        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkQueue queue);
        VkCommandBuffer  BeginSingleTimeCommands();
        void EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue);
        void CreateVertexBuffer(std::vector<Vertex>& vertices, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory);
        void CreateIndexBuffer(std::vector<uint16_t>& indices, VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory);
        void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);
        void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
        VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	};


}


#endif 
#pragma once

namespace tde {
	class Model {
		VkBuffer vertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
		uint32_t vertexCount = 0;

		VkBuffer indexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;
		uint32_t indexCount = 0;

		Renderer* renderer = nullptr;

	public:
		Model();

		Model(const Model& other);

		Model& operator= (Model&& other) noexcept;

		Model(Renderer* _renderer, std::vector<tde::Vertex>& vertices, std::vector<uint16_t>& indices);

		void Draw();
		void Draw(mat4_t& transform);

		void Destroy();

		~Model();


		static std::vector<tde::Vertex> cube_verts;
		static std::vector<uint16_t> cube_indices;
	};
}

namespace tde {
	struct Mesh {
		std::vector<vec3_t> vertices;
		std::vector<vec3_t> normals;
		std::vector<vec3_t> colors;
		std::vector<vec2_t> uvs;
		std::vector<uint16_t> indices;
	};

	void MeshToVertices(const Mesh& mesh, std::vector<tde::Vertex>& vertices);
	


} 
#pragma once
#ifndef GAME_ENGINE
#define GAME_ENGINE

#include <thread>
#include <atomic>
#include <iostream>
#include <chrono>
#include <map>
#include <functional>
#include <string>
#include <cassert>


//mybe use this?
#define PRINT_TO_CONSOLE

#ifdef _DEBUG
#define PRINT(x) std::cout << x << std::endl;
#else
#define PRINT(x) (void)0
#endif

//Time and Input
namespace tde {

	//https://stackoverflow.com/questions/6691555/converting-narrow-string-to-wide-string
	inline std::wstring convert(const std::string& as);

	struct Time {
		static float deltaTime;
		static float fixedDeltaTime;
		static float time;
	};



	uint32_t constexpr numKeys = 256;
	enum KeyCode {
		NONE,
		A,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,
		Alpha0,
		Alpha1,
		Alpha2,
		Alpha3,
		Alpha4,
		Alpha5,
		Alpha6,
		Alpha7,
		Alpha8,
		Alpha9,
		UP, DOWN, LEFT, RIGHT,
		SPACE, TAB, SHIFT, CTRL, INS, DEL, HOME, END, PGUP, PGDN,
		BACK, ESCAPE, RETURN, ENTER, PAUSE, SCROLL,

		OEM_1, OEM_2, OEM_3, OEM_4, OEM_5, OEM_6, OEM_7, OEM_8,
		Mouse0, //left
		Mouse1,	//right... should be middle?
		Mouse2,	//middle
		Mouse3,	//back
		Mouse4,	//fwd
	};

	struct Input //can I make this struct thing static? make two, one internal... or just have the rest be static/global
	{

		static bool newKeyState[numKeys];
		static bool oldKeyState[numKeys];
		static int mouseX;
		static int mouseY;
		static int mousePrevX;
		static int mousePrevY;
		static int mouseDeltaX;
		static int mouseDeltaY;

		/*!
		Is the key pressed currently
		*/
		static bool GetKey(KeyCode keyCode) {
			return newKeyState[keyCode];
		}
		/*!
		Was the key pressed down this frame
		*/
		static bool GetKeyDown(KeyCode keyCode) {
			return newKeyState[keyCode] && !oldKeyState[keyCode];
		}
		/*!
		Was the key released down this frame
		*/
		static bool GetKeyUp(KeyCode keyCode) {
			return !newKeyState[keyCode] && oldKeyState[keyCode];
		}

		//ALL INTERNALS COULD BE MOVE TO AN INTERNAL_INPUT STRUCT OR JUST BE GLOBAL STATIC FUNCS
		//Internal dont call
		static void UpdateKeyState(int keyCode, bool state) {
			newKeyState[keyCode] = state;
		}

		//Internal dont call
		static void UpdateKeys() {
			for (uint32_t i = 0; i < numKeys; i++)
			{
				oldKeyState[i] = newKeyState[i];
			}
			mouseDeltaX = 0;
			mouseDeltaY = 0;
			mousePrevX = mouseX;
			mousePrevY = mouseY;
		}

		//Internal dont call
		static void UpdateMousePos(int xPos, int yPos) {

			mouseX = xPos;
			mouseY = yPos;

			mouseDeltaX = mousePrevX - mouseX;
			mouseDeltaY = mousePrevY - mouseY;

		}

	};


}

//Application and Platforn decl
namespace tde {

	class Application; //HELLLOOOO!!!! I FORWAED DECLARED IT AS I SHOULD STUPID LINKER CAN YOU FIND IT ITS IN THE SAME FILE EVEN DONT BE STUPID PLEASE

	class Platform {
	public:
		virtual ~Platform() = default;
		virtual TdeResult CreateWindowPane(int width, int height, bool fullScreen) = 0;
		virtual TdeResult StartSystemEventLoop() = 0;
		virtual TdeResult ProcessSystemEvents() = 0;
		//virtual Renderer* CreateRenderer(int width, int height) = 0;
		virtual TdeResult SetWindowTitle(const std::string& s) = 0;
		static Application* app;
	};






	//static std::unique_ptr<Renderer> renderer;

	class Application {

		int width = 1920;
		int height = 1080;
		bool fullScreen = false;
		bool drawFrame = false;
		bool running = false;
		std::unique_ptr<Platform> platform;

		//std::function<void(int, int)> callback;

	public:
		std::unique_ptr<Renderer> renderer;
		std::string appName = "App Name Default";

		Application();
		~Application();

		TdeResult Create(int width, int height, bool fullScreen = false);

		TdeResult Start();

		auto getCurrentTime() {
			return std::chrono::high_resolution_clock::now();
		}


		void GameLoop();

		virtual void Init() = 0;
		virtual void FixedUpdate(float deltaTime);//make override frivillig?
		virtual void Update(float deltaTime) = 0;
		virtual void Render(float deltaTime, float extrapolation) = 0;
		virtual void Cleanup();

		void ProcessInput();

		//INTERNAL FUNCTIONS THAT ARE CALLED BY INTERNAL THINGS
		//this should be called "internally"... not by user (I think olc prefixes these with olc_ to differentiate)
		void OnSizeChanged(int newWidth, int newHeight);
		void OnClose();

		//END OF INTERNAL FUNCTIONS

		int GetScreenWidth();
		int GetScreenHeight();
		void ConfigurePlatform(); //end of file because the platforms are not defined otherwise (no .h header files...)
		Renderer& GetRenderer();
	};



}

/*
*	WINDOWS SPECIFICS
*
*/
#ifdef _WIN32
#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include <windowsx.h> // FOR SOME MACROS-.-
namespace tde {

	static std::map<size_t, uint8_t> keyMap; //nont like this being static but meh... we only run at one platform at a time
	class WindowsPlatform : public Platform {


		HWND hwnd = nullptr;

	public:

		virtual TdeResult CreateWindowPane(int width, int height, bool fullScreen) override;

		//callbacks from windows event queue, we deal with them here as they come in
		static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		//this need to be in its own thread as it blocks, ProcessSystemEvents maybe could use peek message and be non blocking?
		TdeResult StartSystemEventLoop() override;

		//non blocking unless we have an endless amount of messages maybe...
		TdeResult ProcessSystemEvents() override;

		TdeResult SetWindowTitle(const std::string& s) override;

	};
}


#endif // !_WIN32
// END OF WINDOWS



#endif // !GAME_ENGINE 
 
#ifdef TOP_DOG_IMPLEMENTATION 

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
	PhysicalDeviceSelector& PhysicalDeviceSelector::set_required_features_12(VkPhysicalDeviceVulkan12Features  features12) {
		return *this;
	}
	PhysicalDeviceSelector& PhysicalDeviceSelector::set_surface(VkSurfaceKHR surface) {
		physicalDevice.surface = surface;
		return *this;
	}
	PhysicalDeviceData PhysicalDeviceSelector::select() {
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
	DeviceBuilder::DeviceBuilder(PhysicalDeviceData physicalDevice) :physicalDevice(physicalDevice) {}

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


		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		//createInfo.pEnabledFeatures = &deviceFeatures;


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
		features.fillModeNonSolid = VK_TRUE;

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

namespace vkinit {
	VkImageCreateInfo image_create_info(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent)
	{
		VkImageCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.pNext = nullptr;

		info.imageType = VK_IMAGE_TYPE_2D;

		info.format = format;
		info.extent = extent;

		info.mipLevels = 1;
		info.arrayLayers = 1;

		//for MSAA. we will not be using it by default, so default it to 1 sample per pixel.
		info.samples = VK_SAMPLE_COUNT_1_BIT;

		//optimal tiling, which means the image is stored on the best gpu format
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.usage = usageFlags;

		return info;
	}

	VkImageViewCreateInfo imageview_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags)
	{
		// build a image-view for the depth image to use for rendering
		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.pNext = nullptr;

		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.image = image;
		info.format = format;
		info.subresourceRange.baseMipLevel = 0;
		info.subresourceRange.levelCount = 1;
		info.subresourceRange.baseArrayLayer = 0;
		info.subresourceRange.layerCount = 1;
		info.subresourceRange.aspectMask = aspectFlags;

		return info;
	}

}

//command pool
namespace vkinit {

	VkCommandPoolCreateInfo vkinit::command_pool_create_info(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags /*= 0*/)
	{
		VkCommandPoolCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.pNext = nullptr;
		info.queueFamilyIndex = queueFamilyIndex;
		info.flags = flags;
		return info;
	}


	VkCommandBufferAllocateInfo vkinit::command_buffer_allocate_info(VkCommandPool pool, uint32_t count /*= 1*/)
	{
		VkCommandBufferAllocateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		info.pNext = nullptr;

		info.commandPool = pool;
		info.commandBufferCount = count;
		info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		return info;
	}

}

// sync objects
namespace vkinit {
	VkFenceCreateInfo fence_create_info(VkFenceCreateFlags flags /*= 0*/)
	{
		VkFenceCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		info.pNext = nullptr;

		info.flags = flags;

		return info;
	}

	VkSemaphoreCreateInfo semaphore_create_info(VkSemaphoreCreateFlags flags /*= 0*/)
	{
		VkSemaphoreCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = flags;
		return info;
	}

}

//cmd buffer+submit+image
namespace vkinit {
	VkCommandBufferBeginInfo vkinit::command_buffer_begin_info(VkCommandBufferUsageFlags flags /*= 0*/)
	{
		VkCommandBufferBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.pNext = nullptr;

		info.pInheritanceInfo = nullptr;
		info.flags = flags;
		return info;
	}


	VkSemaphoreSubmitInfo vkinit::semaphore_submit_info(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore)
	{
		VkSemaphoreSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.semaphore = semaphore;
		submitInfo.stageMask = stageMask;
		submitInfo.deviceIndex = 0;
		submitInfo.value = 1;

		return submitInfo;
	}

	VkCommandBufferSubmitInfo vkinit::command_buffer_submit_info(VkCommandBuffer cmd)
	{
		VkCommandBufferSubmitInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		info.pNext = nullptr;
		info.commandBuffer = cmd;
		info.deviceMask = 0;

		return info;
	}

	VkSubmitInfo2 vkinit::submit_info(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo,
		VkSemaphoreSubmitInfo* waitSemaphoreInfo)
	{
		VkSubmitInfo2 info = {};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		info.pNext = nullptr;

		info.waitSemaphoreInfoCount = waitSemaphoreInfo == nullptr ? 0 : 1;
		info.pWaitSemaphoreInfos = waitSemaphoreInfo;

		info.signalSemaphoreInfoCount = signalSemaphoreInfo == nullptr ? 0 : 1;
		info.pSignalSemaphoreInfos = signalSemaphoreInfo;

		info.commandBufferInfoCount = 1;
		info.pCommandBufferInfos = cmd;

		return info;
	}
	VkImageSubresourceRange vkinit::image_subresource_range(VkImageAspectFlags aspectMask)
	{
		VkImageSubresourceRange subImage{};
		subImage.aspectMask = aspectMask;
		subImage.baseMipLevel = 0;
		subImage.levelCount = VK_REMAINING_MIP_LEVELS;
		subImage.baseArrayLayer = 0;
		subImage.layerCount = VK_REMAINING_ARRAY_LAYERS;

		return subImage;
	}

}


//vk init pipline
namespace vkinit {


	VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_info(VkShaderStageFlagBits stage, VkShaderModule shaderModule) {

		VkPipelineShaderStageCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		info.pNext = nullptr;

		//shader stage
		info.stage = stage;
		//module containing the code for this shader stage
		info.module = shaderModule;
		//the entry point of the shader
		info.pName = "main";
		return info;
	}

	VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info() {
		VkPipelineVertexInputStateCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		info.pNext = nullptr;

		info.vertexBindingDescriptionCount = 0;
		info.vertexAttributeDescriptionCount = 0;
		return info;
	}


	VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info(VkPrimitiveTopology topology) {
		VkPipelineInputAssemblyStateCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		info.pNext = nullptr;

		info.topology = topology;
		//we are not going to use primitive restart on the entire tutorial so leave it on false
		info.primitiveRestartEnable = VK_FALSE;
		return info;
	}

	VkPipelineRasterizationStateCreateInfo rasterization_state_create_info(VkPolygonMode polygonMode, float lineWidth, VkCullModeFlags cullMode, VkFrontFace frontFace)
	{
		VkPipelineRasterizationStateCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		info.pNext = nullptr;

		//discards all primitives before the rasterization stage if enabled which we don't want
		info.rasterizerDiscardEnable = VK_FALSE;

		info.polygonMode = polygonMode;
		info.lineWidth = lineWidth;

		info.cullMode = cullMode;
		info.frontFace = frontFace;

		//no depth bias
		info.depthBiasEnable = VK_FALSE;
		info.depthBiasConstantFactor = 0.0f;
		info.depthClampEnable = VK_FALSE;
		info.depthBiasClamp = 0.0f;
		info.depthBiasSlopeFactor = 0.0f;

		return info;
	}

	VkPipelineMultisampleStateCreateInfo multisampling_state_create_info()
	{
		VkPipelineMultisampleStateCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		info.pNext = nullptr;

		info.sampleShadingEnable = VK_FALSE;
		//multisampling defaulted to no multisampling (1 sample per pixel)
		info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		info.minSampleShading = 1.0f;
		info.pSampleMask = nullptr;
		info.alphaToCoverageEnable = VK_FALSE;
		info.alphaToOneEnable = VK_FALSE;
		return info;
	}

	VkPipelineColorBlendAttachmentState color_blend_attachment_state() {
		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		return colorBlendAttachment;
	}

	VkPipelineLayoutCreateInfo pipeline_layout_create_info() {
		VkPipelineLayoutCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.pNext = nullptr;

		//empty defaults
		info.flags = 0;
		info.setLayoutCount = 0;
		info.pSetLayouts = nullptr;
		info.pushConstantRangeCount = 0;
		info.pPushConstantRanges = nullptr;
		return info;
	}

	VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info(bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp)
	{
		VkPipelineDepthStencilStateCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		info.pNext = nullptr;

		info.depthTestEnable = bDepthTest ? VK_TRUE : VK_FALSE;
		info.depthWriteEnable = bDepthWrite ? VK_TRUE : VK_FALSE;
		info.depthCompareOp = bDepthTest ? compareOp : VK_COMPARE_OP_ALWAYS;
		info.depthBoundsTestEnable = VK_FALSE;
		info.minDepthBounds = 0.0f; // Optional
		info.maxDepthBounds = 1.0f; // Optional
		info.stencilTestEnable = VK_FALSE;

		return info;
	}

	VkRenderingAttachmentInfo vkinit::attachment_info(VkImageView view, VkClearValue* clear, VkImageLayout layout /*= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL*/)
	{
		VkRenderingAttachmentInfo colorAttachment{};
		colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		colorAttachment.pNext = nullptr;

		colorAttachment.imageView = view;
		colorAttachment.imageLayout = layout;
		colorAttachment.loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		if (clear) {
			colorAttachment.clearValue = *clear;
		}

		return colorAttachment;
	}


}

VkRenderingInfo vkinit::rendering_info(VkExtent2D renderExtent, VkRenderingAttachmentInfo* colorAttachment, VkRenderingAttachmentInfo* depthAttachment)
{
	VkRenderingInfo renderInfo{};
	renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderInfo.pNext = nullptr;

	renderInfo.renderArea = VkRect2D{ VkOffset2D { 0, 0 }, renderExtent };
	renderInfo.layerCount = 1;
	renderInfo.colorAttachmentCount = 1;
	renderInfo.pColorAttachments = colorAttachment;
	renderInfo.pDepthAttachment = depthAttachment;
	renderInfo.pStencilAttachment = nullptr;

	return renderInfo;
}

VkRenderingAttachmentInfo vkinit::depth_attachment_info(VkImageView view, VkImageLayout layout /*= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL*/)
{
	VkRenderingAttachmentInfo depthAttachment{};
	depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	depthAttachment.pNext = nullptr;

	depthAttachment.imageView = view;
	depthAttachment.imageLayout = layout;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.clearValue.depthStencil.depth = 0.0f;

	return depthAttachment;
}
 



VkFormat vkutil::FindSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }

        throw std::runtime_error("failed to find supported format!");
    }
    return VK_FORMAT_UNDEFINED;//to remove a warning, maybe throw?
}

void vkutil::transition_image(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout)
{
    VkImageMemoryBarrier2 imageBarrier{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
    imageBarrier.pNext = nullptr;

    imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
    imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

    imageBarrier.oldLayout = currentLayout;
    imageBarrier.newLayout = newLayout;


    VkImageAspectFlags aspectMask = 0;
    aspectMask |= (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : 0;
    aspectMask |= (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

    //if (vkutil::HasStencilComponent(format)) {
    //    aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    //}


    imageBarrier.subresourceRange = vkinit::image_subresource_range(aspectMask);
    imageBarrier.image = image;

    VkDependencyInfo depInfo{};
    depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    depInfo.pNext = nullptr;

    depInfo.imageMemoryBarrierCount = 1;
    depInfo.pImageMemoryBarriers = &imageBarrier;

    vkCmdPipelineBarrier2(cmd, &depInfo);
}

namespace vkutil {

    std::vector<char> read_file(const char* filePath) {
        std::ifstream file(filePath, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            //return nullptr;
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t)file.tellg();

        std::vector<char> buffer(fileSize);

        // put file cursor at beginning
        file.seekg(0);

        // load the entire file into the buffer
        file.read((char*)buffer.data(), fileSize);

        // now that the file is loaded into the buffer, we can close it
        file.close();

        return buffer;
    }

    bool load_shader_module(const std::span<char> data, VkDevice device, VkShaderModule* outShaderModule)
    {

        // create a new shader module, using the buffer we loaded
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.pNext = nullptr;

        // codeSize has to be in bytes, so multply the ints in the buffer by size of
        // int to know the real size of the buffer
        createInfo.codeSize = data.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(data.data());

        // check that the creation goes well.
        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            return false;
        }
        *outShaderModule = shaderModule;
        return true;
    }

} 

//PipelineLayoutBuilder
namespace tde
{
	PipelineLayoutBuilder& PipelineLayoutBuilder::add_push_constants(const VkPushConstantRange& range) {
		pushConstants.push_back(range);
		return *this;
	}
	PipelineLayoutBuilder& PipelineLayoutBuilder::add_descriptor_set_layout(const VkDescriptorSetLayout& descriptor_set_layout) {
		descriptorLayouts.push_back(descriptor_set_layout);
		return *this;
	}

	VkPipelineLayout PipelineLayoutBuilder::build(const VkDevice device) {
		VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::pipeline_layout_create_info();

		if (descriptorLayouts.size() > 0) {
			pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(descriptorLayouts.size());
			pipeline_layout_info.pSetLayouts = descriptorLayouts.data();
		}
		if (pushConstants.size() > 0) {
			pipeline_layout_info.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
			pipeline_layout_info.pPushConstantRanges = pushConstants.data();
		}


		VkPipelineLayout pipelineLayout;
		if (vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &pipelineLayout) != VK_SUCCESS) {
			std::cout << "failed to create pipeline layout\n";
			return VK_NULL_HANDLE; 
		}
		return pipelineLayout;
	}
}

//PipelineBuilder
namespace tde {
	tde::PipelineBuilder::PipelineBuilder() {
		clear();
	}

	VkPipeline tde::PipelineBuilder::build_pipeline(VkDevice device) {
		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.pNext = nullptr;

		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		//setup dummy color blending. We aren't using transparent objects yet
		//the blending is just "no blend", but we do write to the color attachment
		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.pNext = nullptr;

		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;


		//dynamic state of Graphics Pipeline, normally the pipeline is fixed, but now we can resize without switching the whole pipeline out
		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		//build the actual pipeline
	//we now use all of the info structs we have been writing into into this one to create the pipeline
		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.pNext = &renderInfo; //OMG

		pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = VK_NULL_HANDLE;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		pipelineInfo.pDepthStencilState = &depthStencil;
		//renderInfo
		
		pipelineInfo.pDynamicState = &dynamicState;

		//it's easy to error out on create graphics pipeline, so we handle it a bit better than the common VK_CHECK case
		VkPipeline newPipeline;
		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) != VK_SUCCESS) {
			std::cout << "failed to create pipeline\n";
			return VK_NULL_HANDLE; // failed to create graphics pipeline
		}
		else
		{
			return newPipeline;
		}
	}

	void tde::PipelineBuilder::clear()
	{
		// clear all of the structs we need back to 0 with their correct stype

		inputAssembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };

		rasterizer = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };

		colorBlendAttachment = {};

		multisampling = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };

		pipelineLayout = {};

		depthStencil = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };

		renderInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };

		vertexInputInfo = vkinit::vertex_input_state_create_info();

		shaderStages.clear();
	}


	tde::PipelineBuilder& tde::PipelineBuilder::set_pipeline_layout(VkPipelineLayout layout)
	{
		pipelineLayout = layout;
		return *this;
	}

	tde::PipelineBuilder& tde::PipelineBuilder::set_shaders(VkShaderModule vertexShader, VkShaderModule fragmentShader)
	{
		shaderStages.clear();
		shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, vertexShader));
		shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader));
		return *this;
	}


	tde::PipelineBuilder& tde::PipelineBuilder::set_input_topology(VkPrimitiveTopology topology) {
		inputAssembly.topology = topology;
		inputAssembly.primitiveRestartEnable = VK_FALSE;
		return *this;
	}

	tde::PipelineBuilder& tde::PipelineBuilder::set_polygon_mode(VkPolygonMode mode, float lineWidth /*= 1.0f*/)
	{
		rasterizer.polygonMode = mode;
		rasterizer.lineWidth = lineWidth;
		return *this;
	}

	tde::PipelineBuilder& tde::PipelineBuilder::set_cull_mode(VkCullModeFlags cullMode, VkFrontFace frontFace)
	{
		rasterizer.cullMode = cullMode;
		rasterizer.frontFace = frontFace;
		return *this;
	}


	tde::PipelineBuilder& tde::PipelineBuilder::set_color_attachment_format(VkFormat format)
	{
		colorAttachmentFormat = format;
		// connect the format to the renderInfo  structure
		renderInfo.colorAttachmentCount = 1;
		renderInfo.pColorAttachmentFormats = &colorAttachmentFormat;
		return *this;
	}

	tde::PipelineBuilder& tde::PipelineBuilder::set_depth_format(VkFormat format)
	{
		renderInfo.depthAttachmentFormat = format;
		return *this;
	}
	tde::PipelineBuilder& tde::PipelineBuilder::disable_depthtest()
	{
		depthStencil.depthTestEnable = VK_FALSE;
		depthStencil.depthWriteEnable = VK_FALSE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_NEVER;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {};
		depthStencil.back = {};
		depthStencil.minDepthBounds = 0.0f;
		depthStencil.maxDepthBounds = 1.0f;
		return *this;
	}

	tde::PipelineBuilder& tde::PipelineBuilder::set_multisampling_none()
	{
		multisampling.sampleShadingEnable = VK_FALSE;
		// multisampling defaulted to no multisampling (1 sample per pixel)
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f;
		multisampling.pSampleMask = nullptr;
		// no alpha to coverage either
		multisampling.alphaToCoverageEnable = VK_FALSE;
		multisampling.alphaToOneEnable = VK_FALSE;
		return *this;
	}


	tde::PipelineBuilder& tde::PipelineBuilder::disable_blending()
	{
		// default write mask
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		// no blending
		colorBlendAttachment.blendEnable = VK_FALSE;
		return *this;
	}

	tde::PipelineBuilder& tde::PipelineBuilder::enable_blending_additive()
	{
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
		return *this;
	}

	tde::PipelineBuilder& tde::PipelineBuilder::enable_blending_alphablend()
	{
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
		return *this;
	}

	tde::PipelineBuilder& tde::PipelineBuilder::set_vertex_description(vkinit::VertexInputDescription description)
	{
		vertexInputDescription = description;
		vertexInputInfo = vkinit::vertex_input_state_create_info();

		vertexInputInfo.pVertexAttributeDescriptions = vertexInputDescription.attributes.data();
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputDescription.attributes.size());

		vertexInputInfo.pVertexBindingDescriptions = vertexInputDescription.bindings.data();
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputDescription.bindings.size());

		vertexInputInfo.flags = vertexInputDescription.flags;
		return *this;
	}

	PipelineBuilder& PipelineBuilder::enable_depthtest(bool depthWriteEnable, VkCompareOp op /*= VK_COMPARE_OP_GREATER_OR_EQUAL*/)
	{
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = depthWriteEnable;
		depthStencil.depthCompareOp = op;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {};
		depthStencil.back = {};
		depthStencil.minDepthBounds = 0.0f;
		depthStencil.maxDepthBounds = 1.0f;
		return *this;
	}

} 

namespace tde {
	Swapchain::Swapchain()
	{
	}

	Swapchain::~Swapchain()
	{
	}

	void Swapchain::SetContext(VkDevice device, VkPhysicalDevice physicalDevice, VkInstance instance, VkSurfaceKHR surface)
	{
		this->device = device;
		this->physicalDevice = physicalDevice;
		this->instance = instance;
		this->surface = surface;
	}

	void Swapchain::Create(int width, int height, bool vsync)
	{
		assert(device);
		assert(physicalDevice);
		assert(instance);
		assert(surface);

		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = vsync ? VK_PRESENT_MODE_FIFO_KHR : chooseSwapPresentMode(swapChainSupport.presentModes);
		extent = chooseSwapExtent(swapChainSupport.capabilities, width, height);


		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1; //at least one more than min to avoid waiting

		//not more than maximum though (0 is no max exist)
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface); //we already found this!!
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		//check if queues are different, then we need to deal with that
		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform; //no transformation like flip or rotate 90

		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //dont blend with other windows

		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;               //clip pixels outside view

		createInfo.oldSwapchain = VK_NULL_HANDLE; //in case of invalidation of current swap chain


		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain!");
		}

		vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
		images.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapchain, &imageCount, images.data());

		colorFormat = surfaceFormat.format;

		imageViews.resize(images.size());
		for (size_t i = 0; i < images.size(); i++) {
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = images[i];

			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = colorFormat;

			//default mapping of colors
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			//no mipmapping or layers
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device, &createInfo, nullptr, &imageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create image views!");
			}
		}

	}

	VkResult Swapchain::AcquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t& imageIndex)
	{
		return vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, presentCompleteSemaphore, (VkFence)nullptr, &imageIndex);
	}

	VkResult Swapchain::QueuePresent(VkQueue queue, uint32_t& imageIndex, VkSemaphore waitSemaphore)
	{
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = NULL;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchain;
		presentInfo.pImageIndices = &imageIndex;
		// Check if a wait semaphore has been specified to wait for before presenting the image
		if (waitSemaphore != VK_NULL_HANDLE)
		{
			presentInfo.pWaitSemaphores = &waitSemaphore;
			presentInfo.waitSemaphoreCount = 1;
		}
		return vkQueuePresentKHR(queue, &presentInfo);
	}

	void Swapchain::Destroy() {
		vkDestroySwapchainKHR(device, swapchain, nullptr);

		// destroy swapchain resources
		for (int i = 0; i < imageViews.size(); i++) {

			vkDestroyImageView(device, imageViews[i], nullptr);
		}
	}
} 

//this spart below should be copied with implementation guard once we get to it



namespace tde {

	inline void vk_check(VkResult err, const char* msg = "Default") {
		if (err) {
			std::cout << msg << " error code: " << err << std::endl;
			abort();//throw?
		}
		
	}

	vkinit::VertexInputDescription Vertex::GetVertexInputDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
		attributeDescriptions.resize(2);
		//vec3

		


		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);
		//vec3
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, normal);

		vkinit::VertexInputDescription desc{};
		desc.bindings.push_back(bindingDescription);
		desc.attributes = attributeDescriptions;
		return desc;
	}


	Renderer::Renderer()
	{

	}

	Renderer::~Renderer()
	{
		Destroy();
	}

	//Creating instance and setting up validation
	void Renderer::Init(int width, int height) {
		this->width = width;
		this->height = height;
		InstanceBuilder ib;
		auto ver = ib
			.set_app_name("TOP_DOG_ENGINE")
			.request_validation_layers(enableValidationLayers)
			.use_default_debug_messenger()
			.require_api_version(1, 3, 0)
			.build();
		instance = ib.instance;
		debugMessenger = ib.debugMessenger;

	}

	void Renderer::Create(std::vector<void*> args, int width, int height) {
		Init(width, height);

#ifdef _WIN32 
		CreateSurfaceOnWindows(static_cast<HWND>(args[0]), static_cast<HINSTANCE>(args[1]));
#elif __linux__
		// linux
#elif __APPLE__
		// Mac OS
#elif __unix__ // all unices, not all compilers
		// Unix
#endif

		//vulkan 1.3 features
		VkPhysicalDeviceVulkan13Features features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
		features.dynamicRendering = true;
		features.synchronization2 = true;

		//vulkan 1.2 features
		VkPhysicalDeviceVulkan12Features features12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
		features12.bufferDeviceAddress = true;
		features12.descriptorIndexing = true;


		PhysicalDeviceSelector selector{ instance, surface };
		PhysicalDeviceData physDevice = selector
			.set_required_features_12(features12) //none of this is accually taken into account -.-
			.set_required_features_13(features)
			.set_minimum_version(1, 3)
			.select();
		physicalDevice = physDevice.physicalDevice;

		DeviceBuilder deviceBuilder{ physDevice };
		device = deviceBuilder.build();
		graphicsQueue = deviceBuilder.graphicsQueue;
		graphicsQueueFamily = deviceBuilder.graphicsQueueFamily;
		presentQueue = deviceBuilder.presentQueue;


		InitCommands();

		InitSyncStructures();
		
		CreateSwapchain(width, height);



		CreateUniformBuffers();

		InitDescriptors();

		InitPipelines();



		std::vector<tde::Vertex> vertices;
		std::vector<uint16_t> indices;

		//std::vector<obj_loader::Vertex> verts;
		//obj_loader::LoadFromFile("cube_triangulated.obj", verts, indices);
		//vertices.clear();
		//for (size_t i = 0; i < verts.size(); i++)
		//{
		//	vertices.push_back({ verts[i].pos,verts[i].normal });
		//}


		


	}

	void Renderer::CreateSurfaceOnWindows(HWND hwnd, HINSTANCE hInstance) {

		VkWin32SurfaceCreateInfoKHR surfaceInfo{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
		surfaceInfo.pNext = VK_NULL_HANDLE;
		surfaceInfo.flags = 0;
		surfaceInfo.hwnd = hwnd;
		surfaceInfo.hinstance = hInstance;

		if (vkCreateWin32SurfaceKHR(instance, &surfaceInfo, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}

	}
	
	void Renderer::CreateUniformBuffers() {
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);

			vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
		}

		mainDeletionQueue.push_function([&]() {
			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				vkDestroyBuffer(device, uniformBuffers[i], nullptr);
				vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
			}
			});
	}


	void Renderer::InitDescriptors() {

		//descriptor layout
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &uboLayoutBinding;

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		//descriptor pool
		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}

		//finally descriptor sets
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSets[i];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;

			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;

			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = nullptr; // Optional
			descriptorWrite.pTexelBufferView = nullptr; // Optional

			vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
		}
	}


	void Renderer::InitPipelines() {
		InitDefaultPipeline();
	}


	//wrap in a namespace
	std::vector<char> vert_shader_data{ 3, 2, 35, 7, 0, 0, 1, 0, 10, 0, 13, 0, 71, 0, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 11, 0, 6, 0, 1, 0, 0, 0, 71, 76, 83, 76, 46, 115, 116, 100, 46, 52, 53, 48, 0, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 9, 0, 0, 0, 0, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 13, 0, 0, 0, 37, 0, 0, 0, 60, 0, 0, 0, 68, 0, 0, 0, 3, 0, 3, 0, 2, 0, 0, 0, -62, 1, 0, 0, 4, 0, 10, 0, 71, 76, 95, 71, 79, 79, 71, 76, 69, 95, 99, 112, 112, 95, 115, 116, 121, 108, 101, 95, 108, 105, 110, 101, 95, 100, 105, 114, 101, 99, 116, 105, 118, 101, 0, 0, 4, 0, 8, 0, 71, 76, 95, 71, 79, 79, 71, 76, 69, 95, 105, 110, 99, 108, 117, 100, 101, 95, 100, 105, 114, 101, 99, 116, 105, 118, 101, 0, 5, 0, 4, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 5, 0, 6, 0, 11, 0, 0, 0, 103, 108, 95, 80, 101, 114, 86, 101, 114, 116, 101, 120, 0, 0, 0, 0, 6, 0, 6, 0, 11, 0, 0, 0, 0, 0, 0, 0, 103, 108, 95, 80, 111, 115, 105, 116, 105, 111, 110, 0, 6, 0, 7, 0, 11, 0, 0, 0, 1, 0, 0, 0, 103, 108, 95, 80, 111, 105, 110, 116, 83, 105, 122, 101, 0, 0, 0, 0, 6, 0, 7, 0, 11, 0, 0, 0, 2, 0, 0, 0, 103, 108, 95, 67, 108, 105, 112, 68, 105, 115, 116, 97, 110, 99, 101, 0, 6, 0, 7, 0, 11, 0, 0, 0, 3, 0, 0, 0, 103, 108, 95, 67, 117, 108, 108, 68, 105, 115, 116, 97, 110, 99, 101, 0, 5, 0, 3, 0, 13, 0, 0, 0, 0, 0, 0, 0, 5, 0, 7, 0, 17, 0, 0, 0, 85, 110, 105, 102, 111, 114, 109, 66, 117, 102, 102, 101, 114, 79, 98, 106, 101, 99, 116, 0, 6, 0, 5, 0, 17, 0, 0, 0, 0, 0, 0, 0, 110, 111, 114, 109, 97, 108, 0, 0, 6, 0, 5, 0, 17, 0, 0, 0, 1, 0, 0, 0, 118, 105, 101, 119, 0, 0, 0, 0, 6, 0, 5, 0, 17, 0, 0, 0, 2, 0, 0, 0, 112, 114, 111, 106, 0, 0, 0, 0, 5, 0, 3, 0, 19, 0, 0, 0, 117, 98, 111, 0, 5, 0, 6, 0, 28, 0, 0, 0, 80, 117, 115, 104, 67, 111, 110, 115, 116, 97, 110, 116, 0, 0, 0, 0, 6, 0, 5, 0, 28, 0, 0, 0, 0, 0, 0, 0, 109, 111, 100, 101, 108, 0, 0, 0, 5, 0, 3, 0, 30, 0, 0, 0, 112, 99, 0, 0, 5, 0, 5, 0, 37, 0, 0, 0, 105, 110, 80, 111, 115, 105, 116, 105, 111, 110, 0, 0, 5, 0, 3, 0, 48, 0, 0, 0, 110, 109, 0, 0, 5, 0, 3, 0, 58, 0, 0, 0, 110, 0, 0, 0, 5, 0, 5, 0, 60, 0, 0, 0, 105, 110, 78, 111, 114, 109, 97, 108, 0, 0, 0, 0, 5, 0, 4, 0, 68, 0, 0, 0, 110, 111, 114, 109, 97, 108, 0, 0, 72, 0, 5, 0, 11, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 72, 0, 5, 0, 11, 0, 0, 0, 1, 0, 0, 0, 11, 0, 0, 0, 1, 0, 0, 0, 72, 0, 5, 0, 11, 0, 0, 0, 2, 0, 0, 0, 11, 0, 0, 0, 3, 0, 0, 0, 72, 0, 5, 0, 11, 0, 0, 0, 3, 0, 0, 0, 11, 0, 0, 0, 4, 0, 0, 0, 71, 0, 3, 0, 11, 0, 0, 0, 2, 0, 0, 0, 72, 0, 4, 0, 17, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 17, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 72, 0, 5, 0, 17, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 4, 0, 17, 0, 0, 0, 1, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 17, 0, 0, 0, 1, 0, 0, 0, 35, 0, 0, 0, 64, 0, 0, 0, 72, 0, 5, 0, 17, 0, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 72, 0, 4, 0, 17, 0, 0, 0, 2, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 17, 0, 0, 0, 2, 0, 0, 0, 35, 0, 0, 0, -128, 0, 0, 0, 72, 0, 5, 0, 17, 0, 0, 0, 2, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 71, 0, 3, 0, 17, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 19, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 19, 0, 0, 0, 33, 0, 0, 0, 0, 0, 0, 0, 72, 0, 4, 0, 28, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 72, 0, 5, 0, 28, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 72, 0, 5, 0, 28, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 71, 0, 3, 0, 28, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 37, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 60, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 71, 0, 4, 0, 68, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 19, 0, 2, 0, 2, 0, 0, 0, 33, 0, 3, 0, 3, 0, 0, 0, 2, 0, 0, 0, 22, 0, 3, 0, 6, 0, 0, 0, 32, 0, 0, 0, 23, 0, 4, 0, 7, 0, 0, 0, 6, 0, 0, 0, 4, 0, 0, 0, 21, 0, 4, 0, 8, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 8, 0, 0, 0, 9, 0, 0, 0, 1, 0, 0, 0, 28, 0, 4, 0, 10, 0, 0, 0, 6, 0, 0, 0, 9, 0, 0, 0, 30, 0, 6, 0, 11, 0, 0, 0, 7, 0, 0, 0, 6, 0, 0, 0, 10, 0, 0, 0, 10, 0, 0, 0, 32, 0, 4, 0, 12, 0, 0, 0, 3, 0, 0, 0, 11, 0, 0, 0, 59, 0, 4, 0, 12, 0, 0, 0, 13, 0, 0, 0, 3, 0, 0, 0, 21, 0, 4, 0, 14, 0, 0, 0, 32, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 14, 0, 0, 0, 15, 0, 0, 0, 0, 0, 0, 0, 24, 0, 4, 0, 16, 0, 0, 0, 7, 0, 0, 0, 4, 0, 0, 0, 30, 0, 5, 0, 17, 0, 0, 0, 16, 0, 0, 0, 16, 0, 0, 0, 16, 0, 0, 0, 32, 0, 4, 0, 18, 0, 0, 0, 2, 0, 0, 0, 17, 0, 0, 0, 59, 0, 4, 0, 18, 0, 0, 0, 19, 0, 0, 0, 2, 0, 0, 0, 43, 0, 4, 0, 14, 0, 0, 0, 20, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 21, 0, 0, 0, 2, 0, 0, 0, 16, 0, 0, 0, 43, 0, 4, 0, 14, 0, 0, 0, 24, 0, 0, 0, 1, 0, 0, 0, 30, 0, 3, 0, 28, 0, 0, 0, 16, 0, 0, 0, 32, 0, 4, 0, 29, 0, 0, 0, 9, 0, 0, 0, 28, 0, 0, 0, 59, 0, 4, 0, 29, 0, 0, 0, 30, 0, 0, 0, 9, 0, 0, 0, 32, 0, 4, 0, 31, 0, 0, 0, 9, 0, 0, 0, 16, 0, 0, 0, 23, 0, 4, 0, 35, 0, 0, 0, 6, 0, 0, 0, 3, 0, 0, 0, 32, 0, 4, 0, 36, 0, 0, 0, 1, 0, 0, 0, 35, 0, 0, 0, 59, 0, 4, 0, 36, 0, 0, 0, 37, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 39, 0, 0, 0, 0, 0, -128, 63, 32, 0, 4, 0, 45, 0, 0, 0, 3, 0, 0, 0, 7, 0, 0, 0, 32, 0, 4, 0, 47, 0, 0, 0, 7, 0, 0, 0, 16, 0, 0, 0, 32, 0, 4, 0, 57, 0, 0, 0, 7, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 36, 0, 0, 0, 60, 0, 0, 0, 1, 0, 0, 0, 32, 0, 4, 0, 67, 0, 0, 0, 3, 0, 0, 0, 35, 0, 0, 0, 59, 0, 4, 0, 67, 0, 0, 0, 68, 0, 0, 0, 3, 0, 0, 0, 54, 0, 5, 0, 2, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, -8, 0, 2, 0, 5, 0, 0, 0, 59, 0, 4, 0, 47, 0, 0, 0, 48, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 57, 0, 0, 0, 58, 0, 0, 0, 7, 0, 0, 0, 65, 0, 5, 0, 21, 0, 0, 0, 22, 0, 0, 0, 19, 0, 0, 0, 20, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 23, 0, 0, 0, 22, 0, 0, 0, 65, 0, 5, 0, 21, 0, 0, 0, 25, 0, 0, 0, 19, 0, 0, 0, 24, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 26, 0, 0, 0, 25, 0, 0, 0, -110, 0, 5, 0, 16, 0, 0, 0, 27, 0, 0, 0, 23, 0, 0, 0, 26, 0, 0, 0, 65, 0, 5, 0, 31, 0, 0, 0, 32, 0, 0, 0, 30, 0, 0, 0, 15, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 33, 0, 0, 0, 32, 0, 0, 0, -110, 0, 5, 0, 16, 0, 0, 0, 34, 0, 0, 0, 27, 0, 0, 0, 33, 0, 0, 0, 61, 0, 4, 0, 35, 0, 0, 0, 38, 0, 0, 0, 37, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 40, 0, 0, 0, 38, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 41, 0, 0, 0, 38, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 42, 0, 0, 0, 38, 0, 0, 0, 2, 0, 0, 0, 80, 0, 7, 0, 7, 0, 0, 0, 43, 0, 0, 0, 40, 0, 0, 0, 41, 0, 0, 0, 42, 0, 0, 0, 39, 0, 0, 0, -111, 0, 5, 0, 7, 0, 0, 0, 44, 0, 0, 0, 34, 0, 0, 0, 43, 0, 0, 0, 65, 0, 5, 0, 45, 0, 0, 0, 46, 0, 0, 0, 13, 0, 0, 0, 15, 0, 0, 0, 62, 0, 3, 0, 46, 0, 0, 0, 44, 0, 0, 0, 65, 0, 5, 0, 21, 0, 0, 0, 49, 0, 0, 0, 19, 0, 0, 0, 24, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 50, 0, 0, 0, 49, 0, 0, 0, 65, 0, 5, 0, 31, 0, 0, 0, 51, 0, 0, 0, 30, 0, 0, 0, 15, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 52, 0, 0, 0, 51, 0, 0, 0, -110, 0, 5, 0, 16, 0, 0, 0, 53, 0, 0, 0, 50, 0, 0, 0, 52, 0, 0, 0, 12, 0, 6, 0, 16, 0, 0, 0, 54, 0, 0, 0, 1, 0, 0, 0, 34, 0, 0, 0, 53, 0, 0, 0, 62, 0, 3, 0, 48, 0, 0, 0, 54, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 55, 0, 0, 0, 48, 0, 0, 0, 84, 0, 4, 0, 16, 0, 0, 0, 56, 0, 0, 0, 55, 0, 0, 0, 62, 0, 3, 0, 48, 0, 0, 0, 56, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 59, 0, 0, 0, 48, 0, 0, 0, 61, 0, 4, 0, 35, 0, 0, 0, 61, 0, 0, 0, 60, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 62, 0, 0, 0, 61, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 63, 0, 0, 0, 61, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 64, 0, 0, 0, 61, 0, 0, 0, 2, 0, 0, 0, 80, 0, 7, 0, 7, 0, 0, 0, 65, 0, 0, 0, 62, 0, 0, 0, 63, 0, 0, 0, 64, 0, 0, 0, 39, 0, 0, 0, -111, 0, 5, 0, 7, 0, 0, 0, 66, 0, 0, 0, 59, 0, 0, 0, 65, 0, 0, 0, 62, 0, 3, 0, 58, 0, 0, 0, 66, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 69, 0, 0, 0, 58, 0, 0, 0, 79, 0, 8, 0, 35, 0, 0, 0, 70, 0, 0, 0, 69, 0, 0, 0, 69, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 62, 0, 3, 0, 68, 0, 0, 0, 70, 0, 0, 0, -3, 0, 1, 0, 56, 0, 1, 0 };
	std::vector<char> frag_shader_data{ 3, 2, 35, 7, 0, 0, 1, 0, 10, 0, 13, 0, 30, 0, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 11, 0, 6, 0, 1, 0, 0, 0, 71, 76, 83, 76, 46, 115, 116, 100, 46, 52, 53, 48, 0, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 7, 0, 4, 0, 0, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 15, 0, 0, 0, 20, 0, 0, 0, 16, 0, 3, 0, 4, 0, 0, 0, 7, 0, 0, 0, 3, 0, 3, 0, 2, 0, 0, 0, -62, 1, 0, 0, 4, 0, 10, 0, 71, 76, 95, 71, 79, 79, 71, 76, 69, 95, 99, 112, 112, 95, 115, 116, 121, 108, 101, 95, 108, 105, 110, 101, 95, 100, 105, 114, 101, 99, 116, 105, 118, 101, 0, 0, 4, 0, 8, 0, 71, 76, 95, 71, 79, 79, 71, 76, 69, 95, 105, 110, 99, 108, 117, 100, 101, 95, 100, 105, 114, 101, 99, 116, 105, 118, 101, 0, 5, 0, 4, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 5, 0, 5, 0, 9, 0, 0, 0, 108, 105, 103, 104, 116, 68, 105, 114, 0, 0, 0, 0, 5, 0, 4, 0, 13, 0, 0, 0, 110, 111, 114, 109, 0, 0, 0, 0, 5, 0, 4, 0, 15, 0, 0, 0, 110, 111, 114, 109, 97, 108, 0, 0, 5, 0, 5, 0, 20, 0, 0, 0, 111, 117, 116, 67, 111, 108, 111, 114, 0, 0, 0, 0, 71, 0, 4, 0, 15, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 20, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 19, 0, 2, 0, 2, 0, 0, 0, 33, 0, 3, 0, 3, 0, 0, 0, 2, 0, 0, 0, 22, 0, 3, 0, 6, 0, 0, 0, 32, 0, 0, 0, 23, 0, 4, 0, 7, 0, 0, 0, 6, 0, 0, 0, 3, 0, 0, 0, 32, 0, 4, 0, 8, 0, 0, 0, 7, 0, 0, 0, 7, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 11, 0, 0, 0, 0, 0, -128, 63, 44, 0, 6, 0, 7, 0, 0, 0, 12, 0, 0, 0, 10, 0, 0, 0, 11, 0, 0, 0, 10, 0, 0, 0, 32, 0, 4, 0, 14, 0, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 14, 0, 0, 0, 15, 0, 0, 0, 1, 0, 0, 0, 23, 0, 4, 0, 18, 0, 0, 0, 6, 0, 0, 0, 4, 0, 0, 0, 32, 0, 4, 0, 19, 0, 0, 0, 3, 0, 0, 0, 18, 0, 0, 0, 59, 0, 4, 0, 19, 0, 0, 0, 20, 0, 0, 0, 3, 0, 0, 0, 44, 0, 7, 0, 18, 0, 0, 0, 21, 0, 0, 0, 11, 0, 0, 0, 11, 0, 0, 0, 11, 0, 0, 0, 11, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 25, 0, 0, 0, 0, 0, 0, 63, 54, 0, 5, 0, 2, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, -8, 0, 2, 0, 5, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 9, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 13, 0, 0, 0, 7, 0, 0, 0, 62, 0, 3, 0, 9, 0, 0, 0, 12, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 16, 0, 0, 0, 15, 0, 0, 0, 12, 0, 6, 0, 7, 0, 0, 0, 17, 0, 0, 0, 1, 0, 0, 0, 69, 0, 0, 0, 16, 0, 0, 0, 62, 0, 3, 0, 13, 0, 0, 0, 17, 0, 0, 0, 62, 0, 3, 0, 20, 0, 0, 0, 21, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 22, 0, 0, 0, 9, 0, 0, 0, 61, 0, 4, 0, 7, 0, 0, 0, 23, 0, 0, 0, 13, 0, 0, 0, -108, 0, 5, 0, 6, 0, 0, 0, 24, 0, 0, 0, 22, 0, 0, 0, 23, 0, 0, 0, -123, 0, 5, 0, 6, 0, 0, 0, 26, 0, 0, 0, 24, 0, 0, 0, 25, 0, 0, 0, -127, 0, 5, 0, 6, 0, 0, 0, 27, 0, 0, 0, 26, 0, 0, 0, 25, 0, 0, 0, 61, 0, 4, 0, 18, 0, 0, 0, 28, 0, 0, 0, 20, 0, 0, 0, -114, 0, 5, 0, 18, 0, 0, 0, 29, 0, 0, 0, 28, 0, 0, 0, 27, 0, 0, 0, 62, 0, 3, 0, 20, 0, 0, 0, 29, 0, 0, 0, -3, 0, 1, 0, 56, 0, 1, 0 };


	void Renderer::InitDefaultPipeline() {
		VkShaderModule triangleFragShader;
		auto frag = frag_shader_data;
		//auto frag = vkutil::read_file("shaders/frag.spv");
		if (!vkutil::load_shader_module(frag, device, &triangleFragShader)) {
			printl("Error when building the triangle fragment shader module");
		}
		else {
			printl("Triangle fragment shader succesfully loaded");
		}
		VkShaderModule triangleVertexShader;
		auto vert = vert_shader_data;
		//auto vert = vkutil::read_file("shaders/vert.spv");
		if (!vkutil::load_shader_module(vert, device, &triangleVertexShader)) {
			printl("Error when building the triangle vertex shader module");
		}
		else {
			printl("Triangle vertex shader succesfully loaded");
		}

		VkPushConstantRange range = {};
		range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		range.offset = 0;
		range.size = sizeof(mat4_t);


		VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::pipeline_layout_create_info();


		pipeline_layout_info.setLayoutCount = 1;
		pipeline_layout_info.pSetLayouts = &descriptorSetLayout;
		pipeline_layout_info.pushConstantRangeCount = 1; // Optional
		pipeline_layout_info.pPushConstantRanges = &range; // Optional


		vk_check(vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &trianglePipelineLayout));


		PipelineBuilder pipelineBuilder;

		pipelineBuilder.set_pipeline_layout(trianglePipelineLayout);
		pipelineBuilder.set_shaders(triangleVertexShader, triangleFragShader);
		pipelineBuilder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		pipelineBuilder.set_polygon_mode(VK_POLYGON_MODE_FILL);
		pipelineBuilder.set_cull_mode(VK_CULL_MODE_FRONT_BIT, VK_FRONT_FACE_CLOCKWISE);
		pipelineBuilder.set_multisampling_none();
		pipelineBuilder.disable_blending();
		//pipelineBuilder.disable_depthtest();
		pipelineBuilder.enable_depthtest(true, VK_COMPARE_OP_LESS_OR_EQUAL);


		pipelineBuilder.set_vertex_description(Vertex::GetVertexInputDescription());

		//connect the image format we will draw into, from draw image
		pipelineBuilder.set_color_attachment_format(swapchain.colorFormat);
		//pipelineBuilder.set_depth_format(VK_FORMAT_UNDEFINED);
		pipelineBuilder.set_depth_format(VK_FORMAT_D32_SFLOAT);
		
		//finally build the pipeline
		trianglePipeline = pipelineBuilder.build_pipeline(device);

		//clean structures
		vkDestroyShaderModule(device, triangleFragShader, nullptr);
		vkDestroyShaderModule(device, triangleVertexShader, nullptr);

		mainDeletionQueue.push_function([&]() {
			vkDestroyPipelineLayout(device, trianglePipelineLayout, nullptr);
			vkDestroyPipeline(device, trianglePipeline, nullptr);
			});
	}

	//vk utils?

	void Renderer::Destroy() {

		vkDeviceWaitIdle(device);

		//for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		//	vkDestroyBuffer(device, uniformBuffers[i], nullptr);
		//	vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
		//}

		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

		// destroy model buffers instead of hardcoded

		//vkDestroyBuffer(device, vertexBuffer, nullptr);
		//vkFreeMemory(device, vertexBufferMemory, nullptr);

		//vkDestroyBuffer(device, indexBuffer, nullptr);
		//vkFreeMemory(device, indexBufferMemory, nullptr);

		mainDeletionQueue.flush();

		DestroySwapchain();

		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

		//vkDestroyRenderPass(device, renderPass, nullptr);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

			vkDestroyFence(device, frames[i].inFlightFence, nullptr);
			vkDestroySemaphore(device, frames[i].renderFinishedSemaphore, nullptr);
			vkDestroySemaphore(device, frames[i].imageAvailableSemaphore, nullptr);
			//vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
			//vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
			//vkDestroyFence(device, inFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(device, commandPool, nullptr);

		vkDestroyDevice(device, nullptr);

		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);
		//PRINTL("**RENDERER DESTRUCT**");
	}

	void Renderer::DestroySwapchain(){
		swapchain.Destroy();
		vkDestroyImageView(device, depthImageView, nullptr);
		vkDestroyImage(device, depthImage, nullptr);
		vkFreeMemory(device, depthImageMemory, nullptr);
	}


	////TODO Use a builder to shrink it a bit
	void Renderer::CreateSwapchain(uint32_t width, uint32_t height) {

		
		VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
		CreateImage(width, height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
		depthImageView = CreateImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
		ImmediateSubmit([&](VkCommandBuffer cmd) { vkutil::transition_image(cmd, depthImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL); });


		
		swapchain.SetContext(device, physicalDevice, instance, surface);
		swapchain.Create(width, height);

	}

	void Renderer::ResizeSwapchain()
	{
		vkDeviceWaitIdle(device);
		DestroySwapchain();
		CreateSwapchain(width, height);
		resize_requested = false;
	}

	void Renderer::InitCommands()
	{

		VkCommandPoolCreateInfo commandPoolInfo = vkinit::command_pool_create_info(graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

		vk_check(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool));

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {


			// allocate the default command buffer that we will use for rendering
			VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(commandPool, 1);

			vk_check(vkAllocateCommandBuffers(device, &cmdAllocInfo, &frames[i].mainCommandBuffer));
		}



		VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(commandPool, 1);
		vk_check(vkAllocateCommandBuffers(device, &cmdAllocInfo, &imCommandBuffer));

	}

	void Renderer::InitSyncStructures() {
		//create syncronization structures
		//one fence to control when the gpu has finished rendering the frame,
		//and 2 semaphores to syncronize rendering with swapchain
		//we want the fence to start signalled so we can wait on it on the first frame
		VkFenceCreateInfo fenceCreateInfo = vkinit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
		VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphore_create_info();

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vk_check(vkCreateFence(device, &fenceCreateInfo, nullptr, &frames[i].inFlightFence));

			vk_check(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frames[i].imageAvailableSemaphore));
			vk_check(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frames[i].renderFinishedSemaphore));
		}

		vk_check(vkCreateFence(device, &fenceCreateInfo, nullptr, &imFence));
		mainDeletionQueue.push_function([=]() { vkDestroyFence(device, imFence, nullptr); });
	}


	void Renderer::SetViewport(int width, int height) {
		this->width = width;
		this->height = height;
		framebufferResized = true;
	}



	VkCommandBuffer& Renderer::GetCommandBuffer() {
		return get_current_frame().mainCommandBuffer;
	}
	
	void Renderer::BeginFrame() {

		if (resize_requested) {
			ResizeSwapchain();
		}



		vk_check(vkWaitForFences(device, 1, &get_current_frame().inFlightFence, true, 1000000000));
		vk_check(vkResetFences(device, 1, &get_current_frame().inFlightFence));

		
		VkResult e = swapchain.AcquireNextImage(get_current_frame().imageAvailableSemaphore, swapchainImageIndex);// vkAcquireNextImageKHR(device, swapchain, 1000000000, get_current_frame().imageAvailableSemaphore, nullptr, &swapchainImageIndex);
		if (e == VK_ERROR_OUT_OF_DATE_KHR) {
			resize_requested = true;
			return;
		}



		VkCommandBuffer cmd = get_current_frame().mainCommandBuffer;

		// now that we are sure that the commands finished executing, we can safely
		// reset the command buffer to begin recording again.
		vk_check(vkResetCommandBuffer(cmd, 0));

		//begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
		VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		//start the command buffer recording
		vk_check(vkBeginCommandBuffer(cmd, &cmdBeginInfo));
		

		vkutil::transition_image(cmd, swapchain.images[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
		//vkutil::transition_image(cmd, swapchain.images[swapchainImageIndex], VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		////make a clear-color from frame number. This will flash ...
		//VkClearColorValue clearValue;
		//float flash = std::abs(std::sin(frameNumber / 120.f));
		//clearValue = { { 0.0f, 0.0f, flash, 1.0f } };

		//clearValue = { { 99.0f / 255.0f, 149.0f / 255.0f, 238.0f/255.0f } }; //cornflower blue?

		//VkImageSubresourceRange clearRange = vkinit::image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);

		////clear image
		//vkCmdClearColorImage(cmd, swapchain.images[swapchainImageIndex], VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);


		//VkClearValue clearDepthValue = {0};
		//VkImageSubresourceRange clearDepthRange = vkinit::image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);
		//vkCmdClearColorImage(cmd, depthImage, VK_IMAGE_LAYOUT_GENERAL, &clearDepthValue, 1, &clearDepthRange);

		//make the swapchain image into presentable mode
		//vkutil::transition_image(cmd, swapchain.images[swapchainImageIndex], VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		vkutil::transition_image(cmd, swapchain.images[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		//vkutil::transition_image(cmd, depthImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

		VkClearValue clearValue{};
		clearValue.color = { { 99.0f / 255.0f, 149.0f / 255.0f, 238.0f / 255.0f } };
		clearValue.color = { { 0, 0, 0 } };
		clearValue.depthStencil = { 1.0f ,0 };

		//dynamic rendering stuff
		VkRenderingAttachmentInfo colorAttachment = vkinit::attachment_info(swapchain.imageViews[swapchainImageIndex], &clearValue, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		colorAttachment.clearValue = { .color {0,0,0} };
		//VkRenderingAttachmentInfo depthAttachment = vkinit::attachment_info(depthImageView, &clearDepthValue, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
		VkRenderingAttachmentInfo depthAttachment = vkinit::depth_attachment_info(depthImageView, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
		depthAttachment.clearValue = { .depthStencil = { 1.0f, 0 } };
		//depthAttachment.clearValue = clearValue;
		

		VkRenderingInfo renderInfo = vkinit::rendering_info(swapchain.extent, &colorAttachment, &depthAttachment);
		vkCmdBeginRendering(cmd, &renderInfo);

		//bind the pipeline
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, trianglePipeline);

		//set dynamic state
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapchain.extent.width);
		viewport.height = static_cast<float>(swapchain.extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(cmd, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapchain.extent;
		vkCmdSetScissor(cmd, 0, 1, &scissor);

		//printl("extent: ", swapchain.extent.width, ", ", swapchain.extent.height);


		//vkCmdClearDepthStencilImage


	}
	
	void Renderer::EndFrame() {
		VkCommandBuffer cmd = get_current_frame().mainCommandBuffer;
		vkCmdEndRendering(cmd);

		vkutil::transition_image(cmd, swapchain.images[swapchainImageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		//vkutil::transition_image(cmd, swapchain.images[swapchainImageIndex], VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		//finalize the command buffer (we can no longer add commands, but it can now be executed)
		vk_check(vkEndCommandBuffer(cmd));


		VkCommandBufferSubmitInfo cmdinfo = vkinit::command_buffer_submit_info(cmd);

		VkSemaphoreSubmitInfo waitInfo = vkinit::semaphore_submit_info(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, get_current_frame().imageAvailableSemaphore);
		VkSemaphoreSubmitInfo signalInfo = vkinit::semaphore_submit_info(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, get_current_frame().renderFinishedSemaphore);

		VkSubmitInfo2 submit = vkinit::submit_info(&cmdinfo, &signalInfo, &waitInfo);

		//submit command buffer to the queue and execute it.
		// _renderFence will now block until the graphic commands finish execution
		vk_check(vkQueueSubmit2(graphicsQueue, 1, &submit, get_current_frame().inFlightFence));

		//present
		VkResult presentResult = swapchain.QueuePresent(graphicsQueue, swapchainImageIndex, get_current_frame().renderFinishedSemaphore);  //vkQueuePresentKHR(graphicsQueue, &presentInfo);
		if (presentResult == VK_ERROR_OUT_OF_DATE_KHR) {
			resize_requested = true;
		}

		frameNumber++;
		currentFrame++;
		if (currentFrame == MAX_FRAMES_IN_FLIGHT) currentFrame = 0;
	}

	void Renderer::SetUniformBuffer(mat4_t a, mat4_t view, mat4_t proj) {
		UniformBufferObject ubo{};
		ubo.model = a;
		ubo.view = view;
		ubo.proj = proj;

		memcpy(uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));
		VkCommandBuffer cmd = get_current_frame().mainCommandBuffer;
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, trianglePipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);
	}

	void Renderer::WaitIdle() {
		vkDeviceWaitIdle(device);
	}


	void Renderer::CreateVertexBuffer(std::vector<Vertex>& vertices, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory) {
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

		CopyBuffer(stagingBuffer, vertexBuffer, bufferSize, graphicsQueue);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	void Renderer::CreateIndexBuffer(std::vector<uint16_t>& indices, VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory) {
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

		CopyBuffer(stagingBuffer, indexBuffer, bufferSize, graphicsQueue);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	uint32_t Renderer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	void Renderer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate buffer memory!");
		}

		vkBindBufferMemory(device, buffer, bufferMemory, 0);
	}
	//buffer helpers
	void Renderer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkQueue queue) {

		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0; // Optional
		copyRegion.dstOffset = 0; // Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		EndSingleTimeCommands(commandBuffer, queue);
	}

	VkCommandBuffer  Renderer::BeginSingleTimeCommands() {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void  Renderer::EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue) {
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(queue);

		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	}

	void Renderer::ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function)
	{
		vk_check(vkResetFences(device, 1, &imFence));
		vk_check(vkResetCommandBuffer(imCommandBuffer, 0));

		VkCommandBuffer cmd = imCommandBuffer;

		VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

		function(cmd);

		VK_CHECK(vkEndCommandBuffer(cmd));

		VkCommandBufferSubmitInfo cmdinfo = vkinit::command_buffer_submit_info(cmd);
		VkSubmitInfo2 submit = vkinit::submit_info(&cmdinfo, nullptr, nullptr);

		// submit command buffer to the queue and execute it.
		//  _renderFence will now block until the graphic commands finish execution
		vk_check(vkQueueSubmit2(graphicsQueue, 1, &submit, imFence));

		vk_check(vkWaitForFences(device, 1, &imFence, true, 9999999999));
	}


	void Renderer::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(device, image, imageMemory, 0);
	}

	VkImageView Renderer::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}

		return imageView;
	}
} 


namespace tde{


	//void createVertexBuffer(std::vector<tde::Vertex>& vertices) {
	//	vertexCount = static_cast<uint32_t>(vertices.size());
	//	renderer->CreateVertexBuffer(vertices, vertexBuffer, vertexBufferMemory);
	//}

	//void createIndexBuffer(std::vector<uint16_t>& indices) {
	//	indexCount = static_cast<uint32_t>(indices.size());
	//	renderer->CreateIndexBuffer(indices, indexBuffer, indexBufferMemory);
	//}

	Model::Model() {}

	Model::Model(const Model& other) {
		vertexBuffer = other.vertexBuffer;
		vertexBufferMemory = other.vertexBufferMemory;
		vertexCount = other.vertexCount;

		indexBuffer = other.indexBuffer;
		indexBufferMemory = other.indexBufferMemory;
		indexCount = other.indexCount;

		renderer = other.renderer;
	}

	Model& Model::operator= (Model&& other) noexcept {
		vertexBuffer = other.vertexBuffer;
		vertexBufferMemory = other.vertexBufferMemory;
		vertexCount = other.vertexCount;

		indexBuffer = other.indexBuffer;
		indexBufferMemory = other.indexBufferMemory;
		indexCount = other.indexCount;

		renderer = other.renderer;

		other.vertexBuffer = VK_NULL_HANDLE;
		other.vertexBufferMemory = VK_NULL_HANDLE;
		other.vertexCount = 0;

		other.indexBuffer = VK_NULL_HANDLE;
		other.indexBufferMemory = VK_NULL_HANDLE;
		other.indexCount = 0;

		other.renderer = nullptr;
		return *this;
	}

	Model::Model(Renderer* _renderer, std::vector<tde::Vertex>& vertices, std::vector<uint16_t>& indices) {
		renderer = _renderer;
		vertexCount = static_cast<uint32_t>(vertices.size());
		renderer->CreateVertexBuffer(vertices, vertexBuffer, vertexBufferMemory);
		indexCount = static_cast<uint32_t>(indices.size());
		renderer->CreateIndexBuffer(indices, indexBuffer, indexBufferMemory);
	}

	void Model::Draw(mat4_t& transform) {
		//I dont like that this need pipelineLayout...
		vkCmdPushConstants(renderer->GetCommandBuffer(), renderer->trianglePipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4_t), &transform); 
		Draw();
	}

	void Model::Draw() {
		VkBuffer vertexBuffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		VkCommandBuffer& commandBuffer = renderer->GetCommandBuffer();
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);

		// vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);


		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indexCount), 1, 0, 0, 0);

		//printl("Model::Draw indexCount = ", static_cast<uint32_t>(indexCount));
	}

	Model::~Model() {
		Destroy();

		//PRINTL("MODEL DESTRUCT**");
	}

	void Model::Destroy() {
		if (!renderer)
			return;

		VkDevice device = renderer->device;
		vkDestroyBuffer(device, vertexBuffer, nullptr);
		vkFreeMemory(device, vertexBufferMemory, nullptr);

		vkDestroyBuffer(device, indexBuffer, nullptr);
		vkFreeMemory(device, indexBufferMemory, nullptr);
		renderer = nullptr;
		//PRINTL("MODEL DESTROY**");
	}
}


void tde::MeshToVertices(const tde::Mesh& mesh, std::vector<tde::Vertex>& vertices) {
	vertices.resize(mesh.vertices.size());
	for (size_t i = 0; i < vertices.size(); i++)
	{
		vertices[i].pos = mesh.vertices[i];
	}
	for (size_t i = 0; i < mesh.normals.size(); i++)
	{
		vertices[i].normal = mesh.normals[i];
	}

	//for (size_t i = 0; i < mesh.uvs.size(); i++)
	//{
	//	vertices[i]. = mesh.uvs[i];
	//}
}


//std::vector<tde::Vertex> tde::Model::cube_verts{
//		{{ 	 1.000f	, 	 1.000f	, 	-1.000f	}, 	{ 	-0.000f	, 	-0.000f	, 	-1.000f	}},
//		{{ 	 1.000f	, 	-1.000f	, 	-1.000f	}, 	{ 	-0.000f	, 	-0.000f	, 	-1.000f	}},
//		{{ 	 1.000f	, 	 1.000f	, 	 1.000f	}, 	{ 	 1.000f	, 	-0.000f	, 	-0.000f	}},
//		{{ 	 1.000f	, 	-1.000f	, 	 1.000f	}, 	{ 	 1.000f	, 	-0.000f	, 	-0.000f	}},
//		{{  -1.000f	, 	 1.000f	, 	-1.000f	}, 	{ 	-0.000f	, 	-0.000f	, 	-1.000f	}},
//		{{ 	-1.000f	, 	-1.000f	, 	-1.000f	}, 	{ 	-1.000f	, 	-0.000f	, 	-0.000f	}},
//		{{ 	-1.000f	, 	 1.000f	, 	 1.000f	}, 	{ 	-1.000f	, 	-0.000f	, 	-0.000f	}},
//		{{ 	-1.000f	, 	-1.000f	, 	 1.000f	}, 	{ 	-0.000f	, 	-1.000f	, 	-0.000f	}},
//};
//std::vector<uint16_t> tde::Model::cube_indices{
//	4, 2, 0,
//	2, 7, 3,
//	6, 5, 7,
//	1, 7, 5,
//	0, 3, 1,
//	4, 1, 5,
//	4, 6, 2,
//	2, 6, 7,
//	6, 4, 5,
//	1, 3, 7,
//	0, 2, 3,
//	4, 0, 1,
//};

std::vector<tde::Vertex> tde::Model::cube_verts{
	{{	0.500	, 	-0.500	, 	-0.500	}, 	{ 	-0.000	, 	-1.000	, 	-0.000	}},
	{{	0.500	, 	-0.500	, 	0.500	}, 	{ 	-0.000	, 	-1.000	, 	-0.000	}},
	{{	0.500	, 	0.500	, 	0.500	}, 	{ 	-0.000	, 	-0.000	, 	1.000	}},
	{{	-0.500	, 	-0.500	, 	-0.500	}, 	{ 	-0.000	, 	-1.000	, 	-0.000	}},
	{{	0.500	, 	0.500	, 	-0.500	}, 	{ 	1.000	, 	-0.000	, 	-0.000	}},
	{{	-0.500	, 	-0.500	, 	0.500	}, 	{ 	-0.000	, 	-1.000	, 	-0.000	}},
	{{	0.500	, 	0.500	, 	-0.500	}, 	{ 	-0.000	, 	-0.000	, 	-1.000	}},
	{{	0.500	, 	-0.500	, 	-0.500	}, 	{ 	-0.000	, 	-0.000	, 	-1.000	}},
	{{	-0.500	, 	0.500	, 	-0.500	}, 	{ 	-0.000	, 	-0.000	, 	-1.000	}},
	{{	-0.500	, 	-0.500	, 	-0.500	}, 	{ 	-0.000	, 	-0.000	, 	-1.000	}},
	{{	-0.500	, 	0.500	, 	-0.500	}, 	{ 	-1.000	, 	-0.000	, 	-0.000	}},
	{{	-0.500	, 	-0.500	, 	-0.500	}, 	{ 	-1.000	, 	-0.000	, 	-0.000	}},
	{{	-0.500	, 	0.500	, 	0.500	}, 	{ 	-1.000	, 	-0.000	, 	-0.000	}},
	{{	-0.500	, 	-0.500	, 	0.500	}, 	{ 	-1.000	, 	-0.000	, 	-0.000	}},
	{{	0.500	, 	0.500	, 	-0.500	}, 	{ 	-0.000	, 	1.000	, 	-0.000	}},
	{{	0.500	, 	0.500	, 	0.500	}, 	{ 	-0.000	, 	1.000	, 	-0.000	}},
	{{	-0.500	, 	0.500	, 	-0.500	}, 	{ 	-0.000	, 	1.000	, 	-0.000	}},
	{{	-0.500	, 	0.500	, 	0.500	}, 	{ 	-0.000	, 	1.000	, 	-0.000	}},
	{{	0.500	, 	-0.500	, 	0.500	}, 	{ 	-0.000	, 	-0.000	, 	1.000	}},
	{{	-0.500	, 	0.500	, 	0.500	}, 	{ 	-0.000	, 	-0.000	, 	1.000	}},
	{{	-0.500	, 	-0.500	, 	0.500	}, 	{ 	-0.000	, 	-0.000	, 	1.000	}},
	{{	0.500	, 	-0.500	, 	-0.500	}, 	{ 	1.000	, 	-0.000	, 	-0.000	}},
	{{	0.500	, 	0.500	, 	0.500	}, 	{ 	1.000	, 	-0.000	, 	-0.000	}},
	{{	0.500	, 	-0.500	, 	0.500	}, 	{ 	1.000	, 	-0.000	, 	-0.000	}},
};
std::vector<uint16_t> tde::Model::cube_indices{
	2	, 	20	, 	18,
	4	, 	23	, 	21,
	16	, 	15	, 	14,
	0	, 	5	, 	3,
	12	, 	11	, 	13,
	8	, 	7	, 	9,
	2	, 	19	, 	20,
	4	, 	22	, 	23,
	16	, 	17	, 	15,
	0	, 	1	, 	5,
	12	, 	10	, 	11,
	8	, 	6	, 	7,
}; 

//Time & Input impl + Platform static app default assignment
namespace tde {

	//we must initialize them outside because C++
	float Time::deltaTime = 0;
	float Time::fixedDeltaTime = 0.02f; //default 
	float Time::time = 0;


	//we must initialize them outside because C++
	bool Input::newKeyState[numKeys];
	bool Input::oldKeyState[numKeys];
	int Input::mouseX = 0;
	int Input::mouseY = 0;
	int Input::mousePrevX = 0;
	int Input::mousePrevY = 0;
	int Input::mouseDeltaX = 0;
	int Input::mouseDeltaY = 0;

	Application* Platform::app = nullptr;
}

//Application impl
namespace tde {

	Application::Application() {
		ConfigurePlatform();
	}
	Application::~Application() {}

	TdeResult Application::Create(int width, int height, bool fullScreen) {
		this->width = width;
		this->height = height;
		this->fullScreen = fullScreen;
		return Success;
	}

	TdeResult Application::Start() {


		renderer = std::make_unique<Renderer>();


		if (platform->CreateWindowPane(width, height, fullScreen) != Success) {
			return Fail;
		}

		running = true;

		GameLoop();



		return Success;
	}

	//https://gameprogrammingpatterns.com/game-loop.html
	void Application::GameLoop() {

		auto start = getCurrentTime();
		auto previous = getCurrentTime();
		float lag = 0.0;

		Init();

		int frameCount = 0;
		float frameTimer = 0;

		while (running)
		{
			auto current = getCurrentTime();

			std::chrono::duration<float> total = current - start;
			Time::time = total.count();

			std::chrono::duration<float> elapsed = current - previous;
			float deltaTime = elapsed.count();

			previous = current;
			lag += deltaTime;

			ProcessInput();

			while (lag >= Time::fixedDeltaTime)
			{
				Time::deltaTime = Time::fixedDeltaTime;
				FixedUpdate(Time::fixedDeltaTime);


				lag -= Time::fixedDeltaTime;
			}
			Time::deltaTime = deltaTime;
			Update(deltaTime);

			renderer->BeginFrame();

			Render(deltaTime, lag / Time::fixedDeltaTime);

			renderer->EndFrame();
			//renderer->Present();

			// update title bar and compute FPS - swiped from olcPixelGameEngine...
			frameTimer += deltaTime;
			frameCount++;
			if (frameTimer >= 1.0f)
			{
				//nLastFPS = frameCount;
				frameTimer -= 1.0f;
				std::string sTitle = "Top Dog Engine - " + appName + " - FPS: " + std::to_string(frameCount);
				platform->SetWindowTitle(sTitle);
				frameCount = 0;
			}


		}
		renderer->WaitIdle();
		Cleanup();
	}

	void Application::FixedUpdate(float deltaTime) {}//make override frivillig?
	void Application::Cleanup() {}//make override frivillig?

	void Application::ProcessInput() {
		Input::UpdateKeys();
		platform->ProcessSystemEvents();
	};

	void Application::OnSizeChanged(int newWidth, int newHeight) {
		width = newWidth;
		height = newHeight;
		//renderer-> we can call the renderer here with the new size to recreate the image
		//PRINT("OnSizeChanged - width: " << newWidth << " height: " << newHeight);
	}

	void Application::OnClose() {
		PRINT("Closing window");
		running = false;
	}
	Renderer& Application::GetRenderer() {
		return *renderer;
	}

	int Application::GetScreenWidth() { return width; }
	int Application::GetScreenHeight() { return height; }




	void Application::ConfigurePlatform() {

		//https://stackoverflow.com/questions/4605842/how-to-identify-platform-compiler-from-preprocessor-macros
#ifdef _WIN32 // note the underscore: without it, it's not msdn official!
		// Windows (x64 and x86)
		platform = std::make_unique<WindowsPlatform>();
#elif __linux__
	// linux
#elif __APPLE__
	// Mac OS
#elif __unix__ // all unices, not all compilers
	// Unix
#endif

		//point back to app for callbacks
		platform->app = this; //what return value???????????
	}

}

#ifdef _WIN32
//Win32Platform
namespace tde {

	TdeResult WindowsPlatform::CreateWindowPane(int width, int height, bool fullScreen) {



		//set up the key map
		keyMap = {};
		keyMap[0x00] = KeyCode::NONE;
		keyMap[0x41] = KeyCode::A; keyMap[0x42] = KeyCode::B; keyMap[0x43] = KeyCode::C; keyMap[0x44] = KeyCode::D; keyMap[0x45] = KeyCode::E;
		keyMap[0x46] = KeyCode::F; keyMap[0x47] = KeyCode::G; keyMap[0x48] = KeyCode::H; keyMap[0x49] = KeyCode::I; keyMap[0x4A] = KeyCode::J;
		keyMap[0x4B] = KeyCode::K; keyMap[0x4C] = KeyCode::L; keyMap[0x4D] = KeyCode::M; keyMap[0x4E] = KeyCode::N; keyMap[0x4F] = KeyCode::O;
		keyMap[0x50] = KeyCode::P; keyMap[0x51] = KeyCode::Q; keyMap[0x52] = KeyCode::R; keyMap[0x53] = KeyCode::S; keyMap[0x54] = KeyCode::T;
		keyMap[0x55] = KeyCode::U; keyMap[0x56] = KeyCode::V; keyMap[0x57] = KeyCode::W; keyMap[0x58] = KeyCode::X; keyMap[0x59] = KeyCode::Y;
		keyMap[0x5A] = KeyCode::Z;

		keyMap[0x30] = KeyCode::Alpha0; keyMap[0x31] = KeyCode::Alpha1; keyMap[0x32] = KeyCode::Alpha2; keyMap[0x33] = KeyCode::Alpha3; keyMap[0x34] = KeyCode::Alpha4;
		keyMap[0x35] = KeyCode::Alpha5; keyMap[0x36] = KeyCode::Alpha6; keyMap[0x37] = KeyCode::Alpha7; keyMap[0x38] = KeyCode::Alpha8; keyMap[0x39] = KeyCode::Alpha9;


		keyMap[VK_UP] = KeyCode::UP; keyMap[VK_DOWN] = KeyCode::DOWN; keyMap[VK_LEFT] = KeyCode::LEFT; keyMap[VK_RIGHT] = KeyCode::RIGHT;

		keyMap[VK_SPACE] = KeyCode::SPACE; keyMap[VK_TAB] = KeyCode::TAB; keyMap[VK_SHIFT] = KeyCode::SHIFT; keyMap[VK_CONTROL] = KeyCode::CTRL;
		keyMap[VK_INSERT] = KeyCode::INS; keyMap[VK_DELETE] = KeyCode::DEL; keyMap[VK_HOME] = KeyCode::HOME;
		keyMap[VK_END] = KeyCode::END; keyMap[VK_PRIOR] = KeyCode::PGUP; keyMap[VK_NEXT] = KeyCode::PGDN;
		keyMap[VK_BACK] = KeyCode::BACK; keyMap[VK_ESCAPE] = KeyCode::ESCAPE; keyMap[VK_RETURN] = KeyCode::ENTER; keyMap[VK_PAUSE] = KeyCode::PAUSE; keyMap[VK_SCROLL] = KeyCode::SCROLL;


		//keyMap[VK_OEM_1] = KeyCode::OEM_1;			// On US and UK keyboards this is the ';:' key
		//keyMap[VK_OEM_2] = KeyCode::OEM_2;			// On US and UK keyboards this is the '/?' key
		//keyMap[VK_OEM_3] = KeyCode::OEM_3;			// On US keyboard this is the '~' key
		//keyMap[VK_OEM_4] = KeyCode::OEM_4;			// On US and UK keyboards this is the '[{' key
		//keyMap[VK_OEM_5] = KeyCode::OEM_5;			// On US keyboard this is '\|' key.
		//keyMap[VK_OEM_6] = KeyCode::OEM_6;			// On US and UK keyboards this is the ']}' key
		//keyMap[VK_OEM_7] = KeyCode::OEM_7;			// On US keyboard this is the single/double quote key. On UK, this is the single quote/@ symbol key
		//keyMap[VK_OEM_8] = KeyCode::OEM_8;			// miscellaneous characters. Varies by keyboard
		//keyMap[VK_OEM_PLUS] = KeyCode::EQUALS;		// the '+' key on any keyboard
		//keyMap[VK_OEM_COMMA] = KeyCode::COMMA;		// the comma key on any keyboard
		//keyMap[VK_OEM_MINUS] = KeyCode::MINUS;		// the minus key on any keyboard
		//keyMap[VK_OEM_PERIOD] = KeyCode::PERIOD;	// the period key on any keyboard
		//keyMap[VK_CAPITAL] = KeyCode::CAPS_LOCK;

		// Register the window class.
		const wchar_t CLASS_NAME[] = L"Sample Window Class";

		WNDCLASS wc = { };

		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.lpfnWndProc = WindowProc;
		wc.cbClsExtra = NULL;
		wc.cbWndExtra = NULL;
		wc.hInstance = GetModuleHandle(nullptr);
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszMenuName = NULL;
		wc.lpszClassName = CLASS_NAME;

		RegisterClass(&wc);


		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);

		//Not really correct
		if (fullScreen) {
			if ((width != (uint32_t)screenWidth) && (height != (uint32_t)screenHeight))
			{
				DEVMODE dmScreenSettings;
				memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
				dmScreenSettings.dmSize = sizeof(dmScreenSettings);
				dmScreenSettings.dmPelsWidth = width;
				dmScreenSettings.dmPelsHeight = height;
				dmScreenSettings.dmBitsPerPel = 32;
				dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
				if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
				{
					if (MessageBox(NULL, L"Fullscreen Mode not supported!\n Switch to window mode?", L"Error", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
					{
						fullScreen = false;
					}
					else
					{
						return Fail;
					}
				}
				screenWidth = width;
				screenHeight = height;
			}
		}


		DWORD dwExStyle;
		DWORD dwStyle;

		if (fullScreen)
		{
			dwExStyle = WS_EX_APPWINDOW;
			dwStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		}
		else
		{
			dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
			dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		}

		RECT windowRect = {
			0L,
			0L,
			fullScreen ? (long)screenWidth : (long)width,
			fullScreen ? (long)screenHeight : (long)height
		};

		AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);


		// Create the window.
		hwnd = CreateWindowEx(
			dwExStyle,                              // Optional window styles.
			CLASS_NAME,                     // Window class
			L"Top Dog Engine",    // Window text
			dwStyle,            // Window style

			// Size and position
			CW_USEDEFAULT, CW_USEDEFAULT,
			windowRect.right - windowRect.left,
			windowRect.bottom - windowRect.top,

			NULL,       // Parent window    
			NULL,       // Menu
			wc.hInstance,  // Instance handle
			NULL        // Additional application data
		);


		if (hwnd == NULL)
		{
			std::cout << "CreateWindowPane error code: " << GetLastError() << std::endl;
			return Fail;
		}

		//renderererereererere->do the thing
		//app->GetRenderer().Init({ hwnd, wc.hInstance }, width, height);

		app->GetRenderer().Create({ hwnd, wc.hInstance }, width, height);


		ShowWindow(hwnd, SW_SHOW);
		//SetForegroundWindow(hwnd);
		//SetFocus(hwnd);
		return Success;
	}

	//callbacks from windows event queue, we deal with them here as they come in
	LRESULT CALLBACK WindowsPlatform::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

		assert(app);

		switch (uMsg)
		{
		case WM_SIZE:
		{
			int width = LOWORD(lParam);  // Macro to get the low-order word.
			int height = HIWORD(lParam); // Macro to get the high-order word.
			app->OnSizeChanged(width, height);
			return 0;
		}
		case WM_KEYDOWN:	Input::UpdateKeyState(keyMap[wParam], true);                      return 0;
		case WM_KEYUP:		Input::UpdateKeyState(keyMap[wParam], false);                     return 0;

		case WM_LBUTTONDOWN:Input::UpdateKeyState(Mouse0, true);                              return 0;
		case WM_LBUTTONUP:	Input::UpdateKeyState(Mouse0, false);                             return 0;
		case WM_RBUTTONDOWN:Input::UpdateKeyState(Mouse1, true);                              return 0;
		case WM_RBUTTONUP:	Input::UpdateKeyState(Mouse1, false);                             return 0;
		case WM_MBUTTONDOWN:Input::UpdateKeyState(Mouse2, true);                              return 0;
		case WM_MBUTTONUP:	Input::UpdateKeyState(Mouse2, false);                             return 0;
		case WM_XBUTTONDOWN:Input::UpdateKeyState(Mouse2 + HIWORD(wParam), true);             return 0;
		case WM_XBUTTONUP:	Input::UpdateKeyState(Mouse2 + HIWORD(wParam), false);            return 0;

		case WM_MOUSEMOVE:
		{
			int xPos = GET_X_LPARAM(lParam);
			int yPos = GET_Y_LPARAM(lParam);
			Input::UpdateMousePos(xPos, yPos);
			return 0;
		}



		case WM_CLOSE:		app->OnClose();                                                   return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			DestroyWindow(hWnd);
			return 0;
		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
		return 0;
		//return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	//this need to be in its own thread as it blocks, ProcessSystemEvents maybe could use peek message and be non blocking?
	TdeResult WindowsPlatform::StartSystemEventLoop() {
		return Success;
	}

	//non blocking unless we have an endless amount of messages maybe...
	TdeResult WindowsPlatform::ProcessSystemEvents() {
		MSG msg = { };
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		return Success;
	}

	TdeResult WindowsPlatform::SetWindowTitle(const std::string& s)
	{
#ifdef UNICODE
		size_t len = s.size() + 1;
		wchar_t* buf = new wchar_t[len];
		swprintf(buf, len, L"%S", s.c_str());
		SetWindowText(hwnd, buf);
		delete[] buf;
#else
		SetWindowText(hwnd, s.c_str());
#endif
		return Success;
	}

}
#endif //_WIN32

//static helper funcs
namespace tde {
	//https://stackoverflow.com/questions/6691555/converting-narrow-string-to-wide-string
	inline std::wstring convert(const std::string& as)
	{
		size_t len = as.size() + 1;
		wchar_t* buf = new wchar_t[len]; //*2+2???
		swprintf(buf, len, L"%S", as.c_str());
		std::wstring rval = buf;
		delete[] buf;
		return rval;
	}
} 
 
#endif //TOP_DOG_IMPLEMENTATION 
