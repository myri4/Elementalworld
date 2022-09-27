#pragma once

#include <vk/Pipeline.h>
#include <vk/Renderpass.h>
#include <vk/Buffer.h>
#include <vk/Descriptors.h>
#include <magic_enum.hpp>

namespace wc {

	namespace {
		std::vector<uint32_t> readFile(const std::string& filename) {
			std::ifstream file(filename, std::ios::ate | std::ios::binary);

			if (!file.is_open())
				WC_ERROR("Cant open file at location {0}", filename.c_str());

			//find what the size of the file is by looking up the location of the cursor
			//because the cursor is at the end, it gives the size directly in bytes
			size_t fileSize = (size_t)file.tellg();

			//spirv expects the buffer to be on uint32, so make sure to reserve a int vector big enough for the entire file
			std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

			//put file cursor at beggining
			file.seekg(0);

			//load the entire file into the buffer
			file.read((char*)buffer.data(), fileSize);

			//now that the file is loaded into the buffer, we can close it
			file.close();

			return buffer;
		}
	}

	class ShaderModuleWC : public RendererObject<VkShaderModule> {
	private:
		std::vector<uint32_t> shaderData;

	public:		
		spirv_cross::EntryPoint entryPoint;

		VkResult Create(const std::string& filename) {
			shaderData = readFile(filename);
			VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };

			//codeSize has to be in bytes, so multply the ints in the buffer by size of int to know the real size of the buffer
			createInfo.codeSize = shaderData.size() * sizeof(uint32_t);
			createInfo.pCode = shaderData.data();

			spirv_cross::Compiler compiler(shaderData);
			entryPoint = compiler.get_entry_points_and_stages()[0];

			return vkCreateShaderModule(VulkanContext::GetDevice(), &createInfo, nullptr, &m_RendererID);
		}

		void Destroy() {
			vkDestroyShaderModule(VulkanContext::GetDevice(), m_RendererID, nullptr);
		}

		VkPipelineShaderStageCreateInfo GetShaderStageCreateInfo(const VkShaderStageFlagBits& stageFlags) {

			VkPipelineShaderStageCreateInfo info = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };

			//shader stage
			info.stage = stageFlags;
			//module containing the code for this shader stage
			info.module = m_RendererID;
			//the entry point of the shader
			info.pName = entryPoint.name.c_str();
			return info;
		}

		VkPushConstantRange getPushConstantRange(const VkShaderStageFlagBits& stageFlags) {
			spirv_cross::Compiler compiler(shaderData);
			spirv_cross::ShaderResources resources = compiler.get_shader_resources();

			VkPushConstantRange range = {};
			range.stageFlags = stageFlags;
			range.offset = 0;
			if (resources.push_constant_buffers.size() > 0) {

				const auto& bufferType = compiler.get_type(resources.push_constant_buffers[0].base_type_id);
				range.size = compiler.get_declared_struct_size(bufferType);
			}

			return range;
		}

		const std::vector<uint32_t>& getBinary() { return shaderData; }
	};

	struct ShaderCreateInfo {
		std::string vertexShader;
		std::string fragmentShader;
		glm::vec2 windowSize;
		wc::RenderPass renderPass;
		wc::VertexInputDescription vertexDescription;
		bool blending = false;
		bool depthTest = true;
		bool invertY = false;
		VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	};

	class Shader {
		wc::Pipeline pipeline;
		wc::PipelineLayout pipelineLayout;
		wc::DescriptorSetLayout descriptorLayout;
	public:
		const wc::Pipeline& getPipeline() const { return pipeline; }
		const wc::PipelineLayout& getPipelineLayout() const { return pipelineLayout; }
		const wc::DescriptorSetLayout& getDescriptorLayout() const { return descriptorLayout; }
		wc::DescriptorSet descriptorSet;

		void Create(const ShaderCreateInfo& createInfo) {
			wc::PipelineBuilder pipelineBuilder;
			std::array<ShaderModuleWC, 2> shaderModules;

			shaderModules[0].Create(createInfo.vertexShader);
			shaderModules[1].Create(createInfo.fragmentShader);

			{ // Reflection
				std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
				std::vector<VkPushConstantRange> ranges;

				for (uint32_t i = 0; i < shaderModules.size(); i++) {
					spirv_cross::Compiler compiler(shaderModules[i].getBinary());
					spirv_cross::ShaderResources resources = compiler.get_shader_resources();
					VkShaderStageFlags shaderStage = VkShaderStageFlagBits(1 << shaderModules[i].entryPoint.execution_model); // shader stage

					for (auto& resource : resources.push_constant_buffers) {
						auto& baseType = compiler.get_type(resource.base_type_id);
						auto bufferSize = compiler.get_declared_struct_size(baseType);

						uint32_t offset = 0;
						if (ranges.size())
							offset = ranges.back().offset + ranges.back().size;

						auto& pushConstantRange = ranges.emplace_back();
						pushConstantRange.stageFlags = shaderStage;
						pushConstantRange.size = bufferSize;
						pushConstantRange.offset = offset;
					}
					

					for (auto& resource : resources.uniform_buffers) {
						bool add = true;
						VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
						uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
						for (auto& layoutBinding : layoutBindings) {
							if (layoutBinding.descriptorType == descriptorType && layoutBinding.binding == binding)
							{
								add = false;
								layoutBinding.stageFlags |= shaderStage;
								break;
							}
						}

						if (add) {
							auto& layoutBinding = layoutBindings.emplace_back();

							const auto& type = compiler.get_type(resource.type_id);

							uint32_t descriptorCount = 1;
							if (type.array[0] > 0) descriptorCount = type.array[0];

							layoutBinding.descriptorType = descriptorType;
							layoutBinding.descriptorCount = descriptorCount;
							layoutBinding.stageFlags = shaderStage;
							layoutBinding.binding = binding;
							layoutBinding.pImmutableSamplers = nullptr;
						}
					}

					for (auto& resource : resources.storage_buffers) {
						bool add = true;
						VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
						uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
						for (auto& layoutBinding : layoutBindings) {
							if (layoutBinding.descriptorType == descriptorType && layoutBinding.binding == binding)
							{
								add = false;
								layoutBinding.stageFlags |= shaderStage;
								break;
							}
						}

						if (add) {
							auto& layoutBinding = layoutBindings.emplace_back();

							const auto& type = compiler.get_type(resource.type_id);

							uint32_t descriptorCount = 1;
							if (type.array[0] > 0) descriptorCount = type.array[0];

							layoutBinding.descriptorType = descriptorType;
							layoutBinding.descriptorCount = descriptorCount;
							layoutBinding.stageFlags = shaderStage;
							layoutBinding.binding = binding;
							layoutBinding.pImmutableSamplers = nullptr;
						}
					}

					if (shaderStage == VK_SHADER_STAGE_FRAGMENT_BIT) {

						for (auto& resource : resources.storage_images) {
							auto& layoutBinding = layoutBindings.emplace_back();

							const auto& type = compiler.get_type(resource.type_id);

							uint32_t descriptorCount = 1;
							if (type.array[0] > 0) descriptorCount = type.array[0];

							layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
							layoutBinding.descriptorCount = descriptorCount;
							layoutBinding.stageFlags = shaderStage;
							layoutBinding.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
							layoutBinding.pImmutableSamplers = nullptr;
						}

						for (auto& resource : resources.sampled_images) {
							auto& layoutBinding = layoutBindings.emplace_back();

							const auto& type = compiler.get_type(resource.type_id);

							uint32_t descriptorCount = 1;
							if (type.array[0] > 0) descriptorCount = type.array[0];

							layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
							layoutBinding.descriptorCount = descriptorCount;
							layoutBinding.stageFlags = shaderStage;
							layoutBinding.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
							layoutBinding.pImmutableSamplers = nullptr;
						}

					}

					pipelineBuilder.shaderStages[i] = shaderModules[i].GetShaderStageCreateInfo((VkShaderStageFlagBits)shaderStage);
				}

				VkDescriptorSetLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };

				layoutInfo.pBindings = layoutBindings.data();
				layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());

				descriptorLayout = wc::descriptorLayoutCache.create_descriptor_layout(layoutInfo);

				wc::descriptorAllocator.allocate(descriptorSet, descriptorLayout);

				VkPipelineLayoutCreateInfo info = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };

				info.flags = 0;
				info.setLayoutCount = 1;
				info.pSetLayouts = descriptorLayout.GetPointer();
				info.pPushConstantRanges = ranges.data();
				info.pushConstantRangeCount = ranges.size();

				pipelineLayout.Create(info);
				
			}

			pipelineBuilder.vertexInputInfo = wc::vertex_input_state_create_info(createInfo.vertexDescription);

			//build viewport and scissor from the swapchain extents
			pipelineBuilder.viewport.x = 0.f;
			pipelineBuilder.viewport.y = createInfo.windowSize.y; // change this to 0 to invert
			pipelineBuilder.viewport.width = createInfo.windowSize.x;
			pipelineBuilder.viewport.height = -createInfo.windowSize.y; // remove the - to invert
			pipelineBuilder.viewport.minDepth = 0.0f;
			pipelineBuilder.viewport.maxDepth = 1.0f;

			if (createInfo.invertY) {
				pipelineBuilder.viewport.y = 0.f;
				pipelineBuilder.viewport.height = createInfo.windowSize.y;
			}

			pipelineBuilder.scissor.offset = { 0, 0 };
			pipelineBuilder.scissor.extent = { (uint32_t)createInfo.windowSize.x, (uint32_t)createInfo.windowSize.y};

			//a single blend attachment with no blending and writing to RGBA
			VkPipelineColorBlendAttachmentState& colorBlend = pipelineBuilder.colorBlendAttachment;
			colorBlend = {};
			colorBlend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlend.blendEnable = createInfo.blending;

			colorBlend.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			colorBlend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorBlend.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlend.alphaBlendOp = VK_BLEND_OP_ADD;

			pipelineBuilder.depthStencil = wc::depth_stencil_create_info(createInfo.depthTest, createInfo.depthTest, VK_COMPARE_OP_LESS_OR_EQUAL);
			pipelineBuilder.topology = createInfo.topology;
			//finally build the pipeline
			pipeline = pipelineBuilder.build_pipeline(createInfo.renderPass, pipelineLayout);

			for (uint32_t i = 0; i < shaderModules.size(); i++) shaderModules[i].Destroy();
		}

		void Bind(const wc::CommandBuffer& cmd) {
			cmd.BindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, 0, pipelineLayout, descriptorSet);
			cmd.BindPipeline(pipeline);
		}
		
		void Destroy() {
			pipeline.Destroy();
			pipelineLayout.Destroy();
		}
	};

	class ComputeShader {
		wc::ComputePipeline pipeline;
		wc::PipelineLayout pipelineLayout;
		wc::DescriptorSetLayout descriptorLayout;
	public:
		const wc::ComputePipeline& getPipeline() const { return pipeline; }
		const wc::PipelineLayout& getPipelineLayout() const { return pipelineLayout; }
		const wc::DescriptorSetLayout& getDescriptorLayout() const { return descriptorLayout; }
		wc::DescriptorSet descriptorSet;

		void Create(const std::string& shaderPath) {
			ShaderModuleWC shaderModule;

			shaderModule.Create(shaderPath);

			{ // Reflection
				std::vector<VkDescriptorSetLayoutBinding> layoutBindings;

					spirv_cross::Compiler compiler(shaderModule.getBinary());
					spirv_cross::ShaderResources resources = compiler.get_shader_resources();
					VkShaderStageFlags shaderStage = VK_SHADER_STAGE_COMPUTE_BIT;


					for (auto& resource : resources.uniform_buffers) {
						bool add = true;
						VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
						uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
						for (auto& layoutBinding : layoutBindings) {
							if (layoutBinding.descriptorType == descriptorType && layoutBinding.binding == binding)
							{
								add = false;
								layoutBinding.stageFlags |= shaderStage;
								break;
							}
						}

						if (add) {
							auto& layoutBinding = layoutBindings.emplace_back();

							const auto& type = compiler.get_type(resource.type_id);

							uint32_t descriptorCount = 1;
							if (type.array[0] > 0) descriptorCount = type.array[0];

							layoutBinding.descriptorType = descriptorType;
							layoutBinding.descriptorCount = descriptorCount;
							layoutBinding.stageFlags = shaderStage;
							layoutBinding.binding = binding;
							layoutBinding.pImmutableSamplers = nullptr;
						}
					}

					for (auto& resource : resources.storage_buffers) {
						bool add = true;
						VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
						uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
						for (auto& layoutBinding : layoutBindings) {
							if (layoutBinding.descriptorType == descriptorType && layoutBinding.binding == binding)
							{
								add = false;
								layoutBinding.stageFlags |= shaderStage;
								break;
							}
						}

						if (add) {
							auto& layoutBinding = layoutBindings.emplace_back();

							const auto& type = compiler.get_type(resource.type_id);

							uint32_t descriptorCount = 1;
							if (type.array[0] > 0) descriptorCount = type.array[0];

							layoutBinding.descriptorType = descriptorType;
							layoutBinding.descriptorCount = descriptorCount;
							layoutBinding.stageFlags = shaderStage;
							layoutBinding.binding = binding;
							layoutBinding.pImmutableSamplers = nullptr;
						}
					}


					for (auto& resource : resources.storage_images) {
						auto& layoutBinding = layoutBindings.emplace_back();

						const auto& type = compiler.get_type(resource.type_id);

						uint32_t descriptorCount = 1;
						if (type.array[0] > 0) descriptorCount = type.array[0];

						layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
						layoutBinding.descriptorCount = descriptorCount;
						layoutBinding.stageFlags = shaderStage;
						layoutBinding.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
						layoutBinding.pImmutableSamplers = nullptr;
					}

					for (auto& resource : resources.sampled_images) {
						auto& layoutBinding = layoutBindings.emplace_back();

						const auto& type = compiler.get_type(resource.type_id);

						uint32_t descriptorCount = 1;
						if (type.array[0] > 0) descriptorCount = type.array[0];

						layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
						layoutBinding.descriptorCount = descriptorCount;
						layoutBinding.stageFlags = shaderStage;
						layoutBinding.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
						layoutBinding.pImmutableSamplers = nullptr;
					}
				

				VkDescriptorSetLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };

				layoutInfo.pBindings = layoutBindings.data();
				layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());

				descriptorLayout = wc::descriptorLayoutCache.create_descriptor_layout(layoutInfo);

				wc::descriptorAllocator.allocate(descriptorSet, descriptorLayout);

				VkPipelineLayoutCreateInfo info = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };

				info.flags = 0;
				info.setLayoutCount = 1;
				info.pSetLayouts = descriptorLayout.GetPointer();

				if (resources.push_constant_buffers.size() > 0) {
					auto& baseType = compiler.get_type(resources.push_constant_buffers[0].base_type_id);
					auto bufferSize = compiler.get_declared_struct_size(baseType);

					VkPushConstantRange range;
					range.offset = 0;
					range.stageFlags = shaderStage;
					range.size = bufferSize;

					info.pPushConstantRanges = &range;
					info.pushConstantRangeCount = 1;
				}

				pipelineLayout.Create(info);
			}

			//finally build the pipeline
			VkComputePipelineCreateInfo createInfo = { VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
			createInfo.stage = shaderModule.GetShaderStageCreateInfo(VK_SHADER_STAGE_COMPUTE_BIT);
			createInfo.layout = pipelineLayout;

			pipeline.Create(createInfo);

			shaderModule.Destroy();
		}

		void Bind(const wc::CommandBuffer& cmd) {
			cmd.BindDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, 0, pipelineLayout, descriptorSet);
			cmd.BindPipeline(pipeline);
		}

		void Destroy() {
			pipeline.Destroy();
			pipelineLayout.Destroy();
		}
	};

}