
#include "stuff.h"

//Pipeline
namespace tde
{
	struct Pipeline {
		VkPipeline pipeline;
		VkPipelineLayout layout;



		void Destroy(VkDevice device) {
			vkDestroyPipelineLayout(device, layout, nullptr);
			vkDestroyPipeline(device, pipeline, nullptr);
		}
	};

}

//PipelineLayoutBuilder
namespace tde
{
	struct PipelineLayoutBuilder {
		std::vector<VkPushConstantRange> pushConstants{};
		std::vector<VkDescriptorSetLayout> descriptorLayouts{};

		PipelineLayoutBuilder& add_push_constants(const VkPushConstantRange& range);

		PipelineLayoutBuilder& add_descriptor_set_layout(const VkDescriptorSetLayout& descriptor_set_layout);

		VkPipelineLayout build(const VkDevice device);



	};
}

//PipelineBuilder
namespace tde {
	class PipelineBuilder {
	public:

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		VkPipelineVertexInputStateCreateInfo vertexInputInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssembly;
		VkViewport viewport;
		VkRect2D scissor;
		VkPipelineRasterizationStateCreateInfo rasterizer;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineMultisampleStateCreateInfo multisampling;
		VkPipelineDepthStencilStateCreateInfo depthStencil;
		VkPipelineLayout pipelineLayout;
		VkPipelineRenderingCreateInfo renderInfo;
		VkFormat colorAttachmentFormat;
		vkinit::VertexInputDescription vertexInputDescription;

		PipelineBuilder();// { clear(); }

		VkPipeline build_pipeline(VkDevice device);

		void clear();

		PipelineBuilder& set_pipeline_layout(VkPipelineLayout layout);
		PipelineBuilder& set_shaders(VkShaderModule vertexShader, VkShaderModule fragmentShader);
		PipelineBuilder& set_polygon_mode(VkPolygonMode mode, float lineWidth = 1.0f);
		PipelineBuilder& set_input_topology(VkPrimitiveTopology topology);
		PipelineBuilder& set_cull_mode(VkCullModeFlags cullMode, VkFrontFace frontFace);
		PipelineBuilder& set_color_attachment_format(VkFormat format);
		PipelineBuilder& set_depth_format(VkFormat format);
		PipelineBuilder& disable_depthtest();
		PipelineBuilder& enable_depthtest(bool depthWriteEnable, VkCompareOp op = VK_COMPARE_OP_GREATER_OR_EQUAL);
		PipelineBuilder& disable_blending();
		PipelineBuilder& set_multisampling_none();
		PipelineBuilder& enable_blending_additive();
		PipelineBuilder& enable_blending_alphablend();
		PipelineBuilder& set_vertex_description(vkinit::VertexInputDescription description);

	};

}