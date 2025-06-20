#include "renderer.h"
#include "init_helper.h"
#include "vk_init.h"
#include "vk_images.h"
#include "Pipeline.h"
#include "Model.h"
#include "obj_loader.h"
#include <source_location>

//this spart below should be copied with implementation guard once we get to it



namespace tde {

	inline void vk_check(VkResult err, const char* msg = "Default", const std::source_location& location = std::source_location::current()) {
		if (err) {
			std::cout << msg << " error code: " << err << " on line: " << location.line() << std::endl;
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
		pipelineBuilder.set_cull_mode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
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



		vkWaitForFences(device, 1, &get_current_frame().inFlightFence, true, 1000000000);

		
		VkResult e = swapchain.AcquireNextImage(get_current_frame().imageAvailableSemaphore, swapchainImageIndex);// vkAcquireNextImageKHR(device, swapchain, 1000000000, get_current_frame().imageAvailableSemaphore, nullptr, &swapchainImageIndex);
		if (e == VK_ERROR_OUT_OF_DATE_KHR) {
			resize_requested = true;
			//ResizeSwapchain();
			return;
		}

		vk_check(vkResetFences(device, 1, &get_current_frame().inFlightFence));


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

		if (resize_requested) {
			//ResizeSwapchain();
			return;
		}


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