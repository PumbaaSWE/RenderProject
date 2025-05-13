#include "renderer.h"
#include "init_helper.h"
#include "vk_init.h"
#include "vk_images.h"
#include "Pipeline.h"
#include "Model.h"
#include "obj_loader.h"

//this spart below should be copied with implementation guard once we get to it



namespace tde {

	Model plane;
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


		plane = Model(this, Model::cube_verts, Model::cube_indices);
		mainDeletionQueue.push_function([&]() {plane.Destroy(); });


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

	void Renderer::InitDefaultPipeline() {
		VkShaderModule triangleFragShader;
		if (!vkutil::load_shader_module("shaders/frag.spv", device, &triangleFragShader)) {
			printl("Error when building the triangle fragment shader module");
		}
		else {
			printl("Triangle fragment shader succesfully loaded");
		}

		VkShaderModule triangleVertexShader;
		if (!vkutil::load_shader_module("shaders/vert.spv", device, &triangleVertexShader)) {
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
	uint32_t swapchainImageIndex;
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

		//Set and fill uniformBuffers
		mat4_t proj = glm::perspective(glm::radians(60.0f), 720.0f / 420.0f, 0.1f, 1000.0f); //this only change when fov or zNear/zFar changes
		proj[1][1] *= -1; //glm is flipped (OpenGL v Vulkan up? y neg up or down?)
	
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		//mat4_t model = glm::rotate()
		model = glm::rotate(model, 60.0f, { 0,1,1 }); // rotate around the y axis
		view = glm::translate(view, {0, 0, -10});
		//view = glm::translate(testmat, {0,-1,10});
		
		
		glm::mat4 testmat = glm::mat4(1.0f);
		//glm::mat4 testmat =  view * proj;
		//testmat = glm::inverse(testmat);
		//testmat = glm::transpose(testmat);

		UniformBufferObject ubo{};
		ubo.model = testmat;
		ubo.view = view;
		ubo.proj = proj;

		memcpy(uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, trianglePipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);


		//PushConstants
		vkCmdPushConstants(cmd, trianglePipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4_t), &model);

		plane.Draw();

		glm::mat4 model2 = glm::mat4(1.0f);
		model2 = glm::translate(model2, { .6, 0, 8 });
		model2 = glm::rotate(model2, 3.0f, { 0,1,0 }); // rotate around the y axis
		vkCmdPushConstants(cmd, trianglePipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4_t), &model2);

		plane.Draw();

		glm::mat4 model3 = glm::mat4(1.0f);
		model3 = glm::translate(model3, { 0, .7, 4 });
		model3 = glm::rotate(model3, 70.0f, { 1,1,0 }); // rotate around the y axis
		vkCmdPushConstants(cmd, trianglePipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4_t), &model3);

		plane.Draw();


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