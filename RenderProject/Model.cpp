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

	void Model::Draw(mat4_t& transform) {
		//I dont like that this need pipelineLayout...
		vkCmdPushConstants(renderer->GetCommandBuffer(), renderer->trianglePipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4_t), &transform); 
		Draw();
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


//std::vector<tde::Vertex> tde::Model::cube_verts{
//		{{ 	 1.000f	, 	 1.000f	, 	-1.000f	}, 	{ 	-0.000f	, 	-0.000f	, 	-1.000f	}},
//		{{ 	 1.000f	, 	-1.000f	, 	-1.000f	}, 	{ 	-0.000f	, 	-0.000f	, 	-1.000f	}},
//		{{ 	 1.000f	, 	 1.000f	, 	 1.000f	}, 	{ 	 1.000f	, 	-0.000f	, 	-0.000f	}},
//		{{ 	 1.000f	, 	-1.000f	, 	 1.000f	}, 	{ 	 1.000f	, 	-0.000f	, 	-0.000f	}},
//		{{  -1.000f	, 	 1.000f	, 	-1.000f	}, 	{ 	-0.000f	, 	-0.000f	, 	-1.000f	}},
//		{{ 	-1.000f	, 	-1.000f	, 	-1.000f	}, 	{ 	-1.000f	, 	-0.000f	, 	-0.000f	}},
//		{{ 	-1.000f	, 	 1.000f	, 	 1.000f	}, 	{ 	-1.000f	, 	-0.000f	, 	-0.000f	}},
//		{{ 	-1.000f	, 	-1.000f	, 	 1.000f	}, 	{ 	-0.000f	, 	-1.000f	, 	-0.000f	}},
//};
//std::vector<uint16_t> tde::Model::cube_indices{
//	4, 2, 0,
//	2, 7, 3,
//	6, 5, 7,
//	1, 7, 5,
//	0, 3, 1,
//	4, 1, 5,
//	4, 6, 2,
//	2, 6, 7,
//	6, 4, 5,
//	1, 3, 7,
//	0, 2, 3,
//	4, 0, 1,
//};

std::vector<tde::Vertex> tde::Model::cube_verts{
	{{	0.500	, 	-0.500	, 	-0.500	}, 	{ 	-0.000	, 	-1.000	, 	-0.000	}},
	{{	0.500	, 	-0.500	, 	0.500	}, 	{ 	-0.000	, 	-1.000	, 	-0.000	}},
	{{	0.500	, 	0.500	, 	0.500	}, 	{ 	-0.000	, 	-0.000	, 	1.000	}},
	{{	-0.500	, 	-0.500	, 	-0.500	}, 	{ 	-0.000	, 	-1.000	, 	-0.000	}},
	{{	0.500	, 	0.500	, 	-0.500	}, 	{ 	1.000	, 	-0.000	, 	-0.000	}},
	{{	-0.500	, 	-0.500	, 	0.500	}, 	{ 	-0.000	, 	-1.000	, 	-0.000	}},
	{{	0.500	, 	0.500	, 	-0.500	}, 	{ 	-0.000	, 	-0.000	, 	-1.000	}},
	{{	0.500	, 	-0.500	, 	-0.500	}, 	{ 	-0.000	, 	-0.000	, 	-1.000	}},
	{{	-0.500	, 	0.500	, 	-0.500	}, 	{ 	-0.000	, 	-0.000	, 	-1.000	}},
	{{	-0.500	, 	-0.500	, 	-0.500	}, 	{ 	-0.000	, 	-0.000	, 	-1.000	}},
	{{	-0.500	, 	0.500	, 	-0.500	}, 	{ 	-1.000	, 	-0.000	, 	-0.000	}},
	{{	-0.500	, 	-0.500	, 	-0.500	}, 	{ 	-1.000	, 	-0.000	, 	-0.000	}},
	{{	-0.500	, 	0.500	, 	0.500	}, 	{ 	-1.000	, 	-0.000	, 	-0.000	}},
	{{	-0.500	, 	-0.500	, 	0.500	}, 	{ 	-1.000	, 	-0.000	, 	-0.000	}},
	{{	0.500	, 	0.500	, 	-0.500	}, 	{ 	-0.000	, 	1.000	, 	-0.000	}},
	{{	0.500	, 	0.500	, 	0.500	}, 	{ 	-0.000	, 	1.000	, 	-0.000	}},
	{{	-0.500	, 	0.500	, 	-0.500	}, 	{ 	-0.000	, 	1.000	, 	-0.000	}},
	{{	-0.500	, 	0.500	, 	0.500	}, 	{ 	-0.000	, 	1.000	, 	-0.000	}},
	{{	0.500	, 	-0.500	, 	0.500	}, 	{ 	-0.000	, 	-0.000	, 	1.000	}},
	{{	-0.500	, 	0.500	, 	0.500	}, 	{ 	-0.000	, 	-0.000	, 	1.000	}},
	{{	-0.500	, 	-0.500	, 	0.500	}, 	{ 	-0.000	, 	-0.000	, 	1.000	}},
	{{	0.500	, 	-0.500	, 	-0.500	}, 	{ 	1.000	, 	-0.000	, 	-0.000	}},
	{{	0.500	, 	0.500	, 	0.500	}, 	{ 	1.000	, 	-0.000	, 	-0.000	}},
	{{	0.500	, 	-0.500	, 	0.500	}, 	{ 	1.000	, 	-0.000	, 	-0.000	}},
};
std::vector<uint16_t> tde::Model::cube_indices{
	2	, 	20	, 	18,
	4	, 	23	, 	21,
	16	, 	15	, 	14,
	0	, 	5	, 	3,
	12	, 	11	, 	13,
	8	, 	7	, 	9,
	2	, 	19	, 	20,
	4	, 	22	, 	23,
	16	, 	17	, 	15,
	0	, 	1	, 	5,
	12	, 	10	, 	11,
	8	, 	6	, 	7,
};