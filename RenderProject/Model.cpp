#include "Model.h"


namespace tde{


	//void createVertexBuffer(std::vector<tde::Vertex>& vertices) {
	//	vertexCount = static_cast<uint32_t>(vertices.size());
	//	renderer->CreateVertexBuffer(vertices, vertexBuffer, vertexBufferMemory);
	//}

	//void createIndexBuffer(std::vector<uint16_t>& indices) {
	//	indexCount = static_cast<uint32_t>(indices.size());
	//	renderer->CreateIndexBuffer(indices, indexBuffer, indexBufferMemory);
	//}

	Model::Model() {}

	Model::Model(const Model& other) {
		vertexBuffer = other.vertexBuffer;
		vertexBufferMemory = other.vertexBufferMemory;
		vertexCount = other.vertexCount;

		indexBuffer = other.indexBuffer;
		indexBufferMemory = other.indexBufferMemory;
		indexCount = other.indexCount;

		renderer = other.renderer;
	}

	Model& Model::operator= (Model&& other) noexcept {
		vertexBuffer = other.vertexBuffer;
		vertexBufferMemory = other.vertexBufferMemory;
		vertexCount = other.vertexCount;

		indexBuffer = other.indexBuffer;
		indexBufferMemory = other.indexBufferMemory;
		indexCount = other.indexCount;

		renderer = other.renderer;

		other.vertexBuffer = VK_NULL_HANDLE;
		other.vertexBufferMemory = VK_NULL_HANDLE;
		other.vertexCount = 0;

		other.indexBuffer = VK_NULL_HANDLE;
		other.indexBufferMemory = VK_NULL_HANDLE;
		other.indexCount = 0;

		other.renderer = nullptr;
		return *this;
	}

	Model::Model(Renderer* _renderer, std::vector<tde::Vertex>& vertices, std::vector<uint16_t>& indices) {
		renderer = _renderer;
		vertexCount = static_cast<uint32_t>(vertices.size());
		renderer->CreateVertexBuffer(vertices, vertexBuffer, vertexBufferMemory);
		indexCount = static_cast<uint32_t>(indices.size());
		renderer->CreateIndexBuffer(indices, indexBuffer, indexBufferMemory);
	}

	void Model::Draw() {
		VkBuffer vertexBuffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		VkCommandBuffer& commandBuffer = renderer->GetCommandBuffer();
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);

		// vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);


		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indexCount), 1, 0, 0, 0);

		//printl("Model::Draw indexCount = ", static_cast<uint32_t>(indexCount));
	}

	Model::~Model() {
		Destroy();

		//PRINTL("MODEL DESTRUCT**");
	}

	void Model::Destroy() {
		if (!renderer)
			return;

		VkDevice device = renderer->device;
		vkDestroyBuffer(device, vertexBuffer, nullptr);
		vkFreeMemory(device, vertexBufferMemory, nullptr);

		vkDestroyBuffer(device, indexBuffer, nullptr);
		vkFreeMemory(device, indexBufferMemory, nullptr);
		renderer = nullptr;
		//PRINTL("MODEL DESTROY**");
	}
}


void tde::MeshToVertices(const tde::Mesh& mesh, std::vector<tde::Vertex>& vertices) {
	vertices.resize(mesh.vertices.size());
	for (size_t i = 0; i < vertices.size(); i++)
	{
		vertices[i].pos = mesh.vertices[i];
	}
	for (size_t i = 0; i < mesh.normals.size(); i++)
	{
		vertices[i].normal = mesh.normals[i];
	}

	//for (size_t i = 0; i < mesh.uvs.size(); i++)
	//{
	//	vertices[i]. = mesh.uvs[i];
	//}
}