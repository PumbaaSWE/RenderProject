#pragma once
#include "init_helper.h"
#include "stuff.h"

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
		VkResult QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore);

	private:
		VkInstance instance = VK_NULL_HANDLE;
		VkDevice device = VK_NULL_HANDLE;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkSurfaceKHR surface = VK_NULL_HANDLE;
	};
}

#ifdef SWAPCHAIN_IMPLEMENTATION


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

	VkResult Swapchain::QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore)
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
#endif // DEBUG