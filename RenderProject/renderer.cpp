#include "renderer.h"
#define INIT_IMPLEMENTATION
#include "init_helper.h"

#define VK_INIT_IMPLEMENTATION
//#include "vk_init.h"

#define VK_IMAGES_IMPLEMENTATION
#include "vk_images.h"

//this spart below should be copied with implementation guard once we get to it

namespace tde {

	inline void vk_check(VkResult err, const char* msg = "Default") {
		if (err) {
			std::cout << msg << " error code: " << err << std::endl;
			abort();//throw?
		}
		
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

		//
		CreateSwapchain(width, height);

		InitCommands();

		InitSyncStructures();
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

	void Renderer::Destroy() {

		vkDeviceWaitIdle(device);

		//for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		//	vkDestroyBuffer(device, uniformBuffers[i], nullptr);
		//	vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
		//}

		//vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		//vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

		// destroy model buffers instead of hardcoded

		//vkDestroyBuffer(device, vertexBuffer, nullptr);
		//vkFreeMemory(device, vertexBufferMemory, nullptr);

		//vkDestroyBuffer(device, indexBuffer, nullptr);
		//vkFreeMemory(device, indexBufferMemory, nullptr);

		DestroySwapchain();

		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

		vkDestroyRenderPass(device, renderPass, nullptr);

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
		vkDestroySwapchainKHR(device, swapchain, nullptr);

		// destroy swapchain resources
		for (int i = 0; i < swapchainImageViews.size(); i++) {

			vkDestroyImageView(device, swapchainImageViews[i], nullptr);
		}
	}


	//TODO Use a builder to shrink it a bit
	void Renderer::CreateSwapchain(uint32_t width, uint32_t height) {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, width, height);

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
		createInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;

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
		swapchainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());

		//needed later
		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;

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
	}

	void Renderer::ResizeSwapchain()
	{
		vkDeviceWaitIdle(device);
		DestroySwapchain();
		CreateSwapchain(width, height);

		resize_requested = false;
	}

	void Renderer::SetViewport(int width, int height) {
		this->width = width;
		this->height = height;
		framebufferResized = true;
	}

	void Renderer::BeginFrame() {

		if (resize_requested) {
			ResizeSwapchain();
		}



		vk_check(vkWaitForFences(device, 1, &get_current_frame().inFlightFence, true, 1000000000));
		vk_check(vkResetFences(device, 1, &get_current_frame().inFlightFence));

		uint32_t swapchainImageIndex;
		VkResult e = vkAcquireNextImageKHR(device, swapchain, 1000000000, get_current_frame().imageAvailableSemaphore, nullptr, &swapchainImageIndex);
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


		vkutil::transition_image(cmd, swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

		//make a clear-color from frame number. This will flash with a 120 frame period.
		VkClearColorValue clearValue;
		float flash = std::abs(std::sin(frameNumber / 120.f));
		clearValue = { { 0.0f, 0.0f, flash, 1.0f } };

		VkImageSubresourceRange clearRange = vkinit::image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);

		//clear image
		vkCmdClearColorImage(cmd, swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);

		//make the swapchain image into presentable mode
		vkutil::transition_image(cmd, swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

		//finalize the command buffer (we can no longer add commands, but it can now be executed)
		vk_check(vkEndCommandBuffer(cmd));


		VkCommandBufferSubmitInfo cmdinfo = vkinit::command_buffer_submit_info(cmd);

		VkSemaphoreSubmitInfo waitInfo = vkinit::semaphore_submit_info(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, get_current_frame().imageAvailableSemaphore);
		VkSemaphoreSubmitInfo signalInfo = vkinit::semaphore_submit_info(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, get_current_frame().renderFinishedSemaphore);

		VkSubmitInfo2 submit = vkinit::submit_info(&cmdinfo, &signalInfo, &waitInfo);

		//submit command buffer to the queue and execute it.
		// _renderFence will now block until the graphic commands finish execution
		vk_check(vkQueueSubmit2(graphicsQueue, 1, &submit, get_current_frame().inFlightFence));

		//prepare present
		// this will put the image we just rendered to into the visible window.
		// we want to wait on the _renderSemaphore for that, 
		// as its necessary that drawing commands have finished before the image is displayed to the user
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.pSwapchains = &swapchain;
		presentInfo.swapchainCount = 1;

		presentInfo.pWaitSemaphores = &get_current_frame().renderFinishedSemaphore;
		presentInfo.waitSemaphoreCount = 1;

		presentInfo.pImageIndices = &swapchainImageIndex;

		
		VkResult presentResult = vkQueuePresentKHR(graphicsQueue, &presentInfo);
		if (presentResult == VK_ERROR_OUT_OF_DATE_KHR) {
			resize_requested = true;
		}


		frameNumber++;
	}
	
}