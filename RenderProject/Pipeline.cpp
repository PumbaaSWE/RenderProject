#include "vk_init.h"
#include "Pipeline.h"

//PipelineLayoutBuilder
namespace tde
{
	PipelineLayoutBuilder& PipelineLayoutBuilder::add_push_constants(const VkPushConstantRange& range) {
		pushConstants.push_back(range);
		return *this;
	}
	PipelineLayoutBuilder& PipelineLayoutBuilder::add_descriptor_set_layout(const VkDescriptorSetLayout& descriptor_set_layout) {
		descriptorLayouts.push_back(descriptor_set_layout);
		return *this;
	}

	VkPipelineLayout PipelineLayoutBuilder::build(const VkDevice device) {
		VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::pipeline_layout_create_info();

		if (descriptorLayouts.size() > 0) {
			pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(descriptorLayouts.size());
			pipeline_layout_info.pSetLayouts = descriptorLayouts.data();
		}
		if (pushConstants.size() > 0) {
			pipeline_layout_info.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
			pipeline_layout_info.pPushConstantRanges = pushConstants.data();
		}


		VkPipelineLayout pipelineLayout;
		if (vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &pipelineLayout) != VK_SUCCESS) {
			std::cout << "failed to create pipeline layout\n";
			return VK_NULL_HANDLE; 
		}
		return pipelineLayout;
	}
}

//PipelineBuilder
namespace tde {
	tde::PipelineBuilder::PipelineBuilder() {
		clear();
	}

	VkPipeline tde::PipelineBuilder::build_pipeline(VkDevice device) {
		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.pNext = nullptr;

		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		//setup dummy color blending. We aren't using transparent objects yet
		//the blending is just "no blend", but we do write to the color attachment
		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.pNext = nullptr;

		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;


		//dynamic state of Graphics Pipeline, normally the pipeline is fixed, but now we can resize without switching the whole pipeline out
		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		//build the actual pipeline
	//we now use all of the info structs we have been writing into into this one to create the pipeline
		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.pNext = nullptr;

		pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = VK_NULL_HANDLE;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.pDepthStencilState = &depthStencil;

		pipelineInfo.pDynamicState = &dynamicState;

		//it's easy to error out on create graphics pipeline, so we handle it a bit better than the common VK_CHECK case
		VkPipeline newPipeline;
		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) != VK_SUCCESS) {
			std::cout << "failed to create pipeline\n";
			return VK_NULL_HANDLE; // failed to create graphics pipeline
		}
		else
		{
			return newPipeline;
		}
	}

	void tde::PipelineBuilder::clear()
	{
		// clear all of the structs we need back to 0 with their correct stype

		inputAssembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };

		rasterizer = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };

		colorBlendAttachment = {};

		multisampling = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };

		pipelineLayout = {};

		depthStencil = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };

		renderInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };


		shaderStages.clear();
	}


	tde::PipelineBuilder& tde::PipelineBuilder::set_pipeline_layout(VkPipelineLayout layout)
	{
		pipelineLayout = layout;
		return *this;
	}

	tde::PipelineBuilder& tde::PipelineBuilder::set_shaders(VkShaderModule vertexShader, VkShaderModule fragmentShader)
	{
		shaderStages.clear();
		shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, vertexShader));
		shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader));
		return *this;
	}


	tde::PipelineBuilder& tde::PipelineBuilder::set_input_topology(VkPrimitiveTopology topology) {
		inputAssembly.topology = topology;
		inputAssembly.primitiveRestartEnable = VK_FALSE;
		return *this;
	}

	tde::PipelineBuilder& tde::PipelineBuilder::set_polygon_mode(VkPolygonMode mode, float lineWidth /*= 1.0f*/)
	{
		rasterizer.polygonMode = mode;
		rasterizer.lineWidth = lineWidth;
		return *this;
	}

	tde::PipelineBuilder& tde::PipelineBuilder::set_cull_mode(VkCullModeFlags cullMode, VkFrontFace frontFace)
	{
		rasterizer.cullMode = cullMode;
		rasterizer.frontFace = frontFace;
		return *this;
	}


	tde::PipelineBuilder& tde::PipelineBuilder::set_color_attachment_format(VkFormat format)
	{
		colorAttachmentFormat = format;
		// connect the format to the renderInfo  structure
		renderInfo.colorAttachmentCount = 1;
		renderInfo.pColorAttachmentFormats = &colorAttachmentFormat;
		return *this;
	}

	tde::PipelineBuilder& tde::PipelineBuilder::set_depth_format(VkFormat format)
	{
		renderInfo.depthAttachmentFormat = format;
		return *this;
	}
	tde::PipelineBuilder& tde::PipelineBuilder::disable_depthtest()
	{
		depthStencil.depthTestEnable = VK_FALSE;
		depthStencil.depthWriteEnable = VK_FALSE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_NEVER;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {};
		depthStencil.back = {};
		depthStencil.minDepthBounds = 0.f;
		depthStencil.maxDepthBounds = 1.f;
		return *this;
	}

	tde::PipelineBuilder& tde::PipelineBuilder::set_multisampling_none()
	{
		multisampling.sampleShadingEnable = VK_FALSE;
		// multisampling defaulted to no multisampling (1 sample per pixel)
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f;
		multisampling.pSampleMask = nullptr;
		// no alpha to coverage either
		multisampling.alphaToCoverageEnable = VK_FALSE;
		multisampling.alphaToOneEnable = VK_FALSE;
		return *this;
	}


	tde::PipelineBuilder& tde::PipelineBuilder::disable_blending()
	{
		// default write mask
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		// no blending
		colorBlendAttachment.blendEnable = VK_FALSE;
		return *this;
	}

	tde::PipelineBuilder& tde::PipelineBuilder::enable_blending_additive()
	{
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
		return *this;
	}

	tde::PipelineBuilder& tde::PipelineBuilder::enable_blending_alphablend()
	{
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
		return *this;
	}

	tde::PipelineBuilder& tde::PipelineBuilder::set_vertex_description(vkinit::VertexInputDescription description)
	{
		vertexInputDescription = description;
		vertexInputInfo = vkinit::vertex_input_state_create_info();

		vertexInputInfo.pVertexAttributeDescriptions = vertexInputDescription.attributes.data();
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputDescription.attributes.size());

		vertexInputInfo.pVertexBindingDescriptions = vertexInputDescription.bindings.data();
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputDescription.bindings.size());

		vertexInputInfo.flags = vertexInputDescription.flags;
		return *this;
	}

	PipelineBuilder& PipelineBuilder::enable_depthtest(bool depthWriteEnable, VkCompareOp op /*= VK_COMPARE_OP_GREATER_OR_EQUAL*/)
	{
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = depthWriteEnable;
		depthStencil.depthCompareOp = op;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {};
		depthStencil.back = {};
		depthStencil.minDepthBounds = 0.f;
		depthStencil.maxDepthBounds = 1.f;
		return *this;
	}

}