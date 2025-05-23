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
		VkResult QueuePresent(VkQueue queue, uint32_t& imageIndex, VkSemaphore waitSemaphore);

	private:
		VkInstance instance = VK_NULL_HANDLE;
		VkDevice device = VK_NULL_HANDLE;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkSurfaceKHR surface = VK_NULL_HANDLE;
	};
}
