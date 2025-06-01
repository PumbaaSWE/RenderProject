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
		void Draw(mat4_t& transform);

		void Destroy();

		~Model();


		static std::vector<tde::Vertex> cube_verts;
		static std::vector<uint16_t> cube_indices;

		static std::vector<tde::Vertex> sphere_verts;
		static std::vector<uint16_t> sphere_indices;

		static std::vector<tde::Vertex> plane_verts;
		static std::vector<uint16_t> plane_indices;
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