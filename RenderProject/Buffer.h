#pragma once



#include "stuff.h"

class Buffer
{
public:
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory deviceMemory = VK_NULL_HANDLE;
	void* mappedData = nullptr;
	VkDeviceSize size = 0;

	void SetData(const void* data, size_t size) {
		memcpy(mappedData, data, size);
	}
};

namespace tdebuffers {

	std::vector<VkBuffer> buffers;
	std::vector<VkDeviceMemory> memories;
	std::vector<VkDeviceSize> sizes;

	struct Buffer
	{
		int id;

		VkBuffer& GetBuffer() {
			return buffers[id];
		}
		VkDeviceSize& GetBufferSize() {
			return sizes[id];
		}

		VkDeviceMemory& GetBufferMemory() {
			return memories[id];
		}
	};


	int AddBuffer(VkBuffer& buf, VkDeviceMemory& mem, VkDeviceSize size) {
		int id = buffers.size();

		buffers.push_back(buf);
		memories.push_back(mem);
		sizes.push_back(size);

		return id;
	}

	VkBuffer& GetBuffer(int id) {
		return buffers[id];
	}

	VkDeviceSize& GetBufferSize(int id) {
		return sizes[id];
	}

	VkDeviceMemory& GetBufferMemory(int id) {
		return memories[id];
	}
}