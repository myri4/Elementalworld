#pragma once

#include "VulkanContext.h"
#include <fstream>
#include <spirv_cross/spirv_cross.hpp>

namespace wc {	

	struct VertexInputDescription {

		std::vector<VkVertexInputBindingDescription> bindings;
		std::vector<VkVertexInputAttributeDescription> attributes;

		VkPipelineVertexInputStateCreateFlags flags = 0;
	};

	struct Pipeline : public RendererObject<VkPipeline> {

		Pipeline() = default;
		Pipeline(const VkPipeline& handle) { m_RendererID = handle; }

		void Destroy() { vkDestroyPipeline(VulkanContext::GetDevice(), m_RendererID, nullptr); }
	};

	struct PipelineCache : public RendererObject<VkPipelineCache> {

		VkResult Create(const VkPipelineCacheCreateInfo& createInfo) {
			return vkCreatePipelineCache(VulkanContext::GetDevice(), &createInfo, nullptr, &m_RendererID);
		}

		void Destroy() {
			vkDestroyPipelineCache(VulkanContext::GetDevice(), m_RendererID, nullptr);
		}

		VkResult MergePipelineCaches(const uint32_t& count, const VkPipelineCache* caches) {
			return vkMergePipelineCaches(VulkanContext::GetDevice(), m_RendererID, count, caches);
		}

		void* GetData() {
			void* data = nullptr;
			vkGetPipelineCacheData(VulkanContext::GetDevice(), m_RendererID, nullptr, data); // @TODO: 4th arg is probably incorrect
			return data;
		}
	};


	struct ComputePipeline : public Pipeline {

		VkResult Create(const VkComputePipelineCreateInfo& pipelineInfo) {
			return vkCreateComputePipelines(VulkanContext::GetDevice(), nullptr, 1, &pipelineInfo, nullptr, &m_RendererID);
		}
	};

	struct PipelineBuilder {
		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
		VkPipelineVertexInputStateCreateInfo vertexInputInfo;
		VkViewport viewport;
		VkRect2D scissor;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineDepthStencilStateCreateInfo depthStencil;
		VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		VkPipeline build_pipeline(const VkRenderPass& pass, const VkPipelineLayout& pipelineLayout) {
			//make viewport state from our stored viewport and scissor.
			//at the moment we wont support multiple viewports or scissors
			VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };

			viewportState.viewportCount = 1;
			viewportState.pViewports = &viewport;
			viewportState.scissorCount = 1;
			viewportState.pScissors = &scissor;

			VkPipelineColorBlendStateCreateInfo colorBlending = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };

			colorBlending.logicOpEnable = false;
			colorBlending.logicOp = VK_LOGIC_OP_COPY;
			colorBlending.attachmentCount = 1;
			colorBlending.pAttachments = &colorBlendAttachment;

			//build the actual pipeline
			//we now use all of the info structs we have been writing into into this one to create the pipeline
			VkGraphicsPipelineCreateInfo pipelineInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

			pipelineInfo.stageCount = shaderStages.size();
			pipelineInfo.pStages = shaderStages.data();
			pipelineInfo.pVertexInputState = &vertexInputInfo;

			VkPipelineInputAssemblyStateCreateInfo inputAssembly = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };

			inputAssembly.topology = topology;
			//we are not going to use primitive restart on the entire tutorial so leave it on false
			inputAssembly.primitiveRestartEnable = false;
			pipelineInfo.pInputAssemblyState = &inputAssembly;

			pipelineInfo.pViewportState = &viewportState;

			//configure the rasterizer to draw filled triangles
			VkPipelineRasterizationStateCreateInfo rasterizer = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };

			rasterizer.depthClampEnable = false;
			//discards all primitives before the rasterization stage if enabled which we don't want
			rasterizer.rasterizerDiscardEnable = false;

			rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizer.lineWidth = 1.0f;
			//no backface cull
			rasterizer.cullMode = VK_CULL_MODE_NONE;
			rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
			//no depth bias
			rasterizer.depthBiasEnable = false;
			rasterizer.depthBiasConstantFactor = 0.0f;
			rasterizer.depthBiasClamp = 0.0f;
			rasterizer.depthBiasSlopeFactor = 0.0f;
			pipelineInfo.pRasterizationState = &rasterizer;

			//we dont use multisampling, so just run the default one
			VkPipelineMultisampleStateCreateInfo multisampling = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };

			multisampling.sampleShadingEnable = false;
			//multisampling defaulted to no multisampling (1 sample per pixel)
			multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			multisampling.minSampleShading = 1.0f;
			multisampling.pSampleMask = nullptr;
			multisampling.alphaToCoverageEnable = false;
			multisampling.alphaToOneEnable = false;
			pipelineInfo.pMultisampleState = &multisampling;

			pipelineInfo.pColorBlendState = &colorBlending;
			pipelineInfo.layout = pipelineLayout;
			pipelineInfo.renderPass = pass;
			pipelineInfo.subpass = 0;
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

			pipelineInfo.pDepthStencilState = &depthStencil;

			//its easy to error out on create graphics pipeline, so we handle it a bit better than the common VK_CHECK case
			VkPipeline newPipeline;
			if (vkCreateGraphicsPipelines(VulkanContext::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) != VK_SUCCESS) {
				WC_ERROR("Failed to create pipline");
				return VK_NULL_HANDLE; // failed to create graphics pipeline
			}
			else			
				return newPipeline;			
		}
	};	
	
	VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info(const VertexInputDescription& desc) {
		VkPipelineVertexInputStateCreateInfo info = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

		info.pVertexAttributeDescriptions = desc.attributes.data();
		info.vertexAttributeDescriptionCount = desc.attributes.size();

		info.pVertexBindingDescriptions = desc.bindings.data();
		info.vertexBindingDescriptionCount = desc.bindings.size();
		return info;
	}

	VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info() {
		VkPipelineVertexInputStateCreateInfo info = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

		info.pVertexAttributeDescriptions = nullptr;
		info.vertexAttributeDescriptionCount = 0;

		info.pVertexBindingDescriptions = nullptr;
		info.vertexBindingDescriptionCount = 0;
		return info;
	}

	struct PipelineLayout : public RendererObject<VkPipelineLayout> {

		VkResult Create(const VkPipelineLayoutCreateInfo& info) {
			return vkCreatePipelineLayout(VulkanContext::GetDevice(), &info, nullptr, &m_RendererID);
		}

		void Destroy() { vkDestroyPipelineLayout(VulkanContext::GetDevice(), m_RendererID, nullptr); }
	};
}