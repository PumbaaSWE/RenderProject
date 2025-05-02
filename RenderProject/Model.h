#pragma once
#include "renderer.h"

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

		~Model();
	};
}