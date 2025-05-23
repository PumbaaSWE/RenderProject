#pragma once
#include "vk_init.h"
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
