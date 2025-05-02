#pragma once

#include "stuff.h"

class Device
{

public:
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VkCommandPool commandPool = VK_NULL_HANDLE;


    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkQueue queue);
    VkCommandBuffer  BeginSingleTimeCommands();
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue);
};

