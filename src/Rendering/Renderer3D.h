#pragma once

#include "../world/Block.h"
#undef near
#undef far

#include <bvh/v2/bvh.h>
#include <bvh/v2/vec.h>
#include <bvh/v2/ray.h>
#include <bvh/v2/node.h>
#include <bvh/v2/default_builder.h>
#include <bvh/v2/thread_pool.h>
#include <bvh/v2/executor.h>
#include <bvh/v2/stack.h>
#include <bvh/v2/tri.h>


#define near
#define far
#include <wc/vk/Buffer.h>
#include <wc/vk/Images.h>
#include <wc/Shader.h>
#include "../Globals.h"
#include <wc/Maths/Camera.h>
#include "AssetManager.h"
#include <stdint.h>
#include "../Settings.h"

namespace wc {

	int GetTreeCount(int maxDepth, int dimensions = 3) {
		int depth = 0;
		int count = 1;
		while (depth < maxDepth) {
			depth++;
			count += pow(pow(2, dimensions), depth);
		}

		return count;
	}

	struct DrawInfo {
		uint32_t vertexOffset = 0;
		uint32_t indexOffset = 0;
	};

	struct DrawCommand {
		uint32_t count;
		uint32_t firstIndex;
		uint32_t bvhID;
		uint32_t baseVertex;
		glm::vec3 transform;
		uint32_t _pad1;
	};

	struct Node {
		glm::vec3 start;
		uint32_t left;
		glm::vec3 end;
		uint32_t right;
		uint32_t first;
		uint32_t count;
		uint32_t flags;
		uint32_t _pad;
	};

	struct ChunkNode {
		ChunkNode() = default;

		glm::vec3 start = glm::vec3(0.f);
		uint32_t isLeaf = false;
		glm::vec3 end = glm::vec3(0.f);
		uint32_t parentID = 0; // not really used in shader just for convinience
		uint32_t children[8] = { 0 };
	};

	struct Light {
		glm::vec3 vector;
		uint32_t color;
	};

	struct SceneData {
		glm::vec3 cameraPos = glm::vec3(0.f);
		uint32_t numLights = 0;
		glm::vec3 lower_left_corner = glm::vec3(0.f);
		uint32_t bvhCounter = 0;
		glm::vec3 horizontal = glm::vec3(0.f);
		uint32_t _pad = 0;
		glm::vec3 vertical = glm::vec3(0.f);
	};

	struct Material {
		uint32_t albedo = 0;
		uint32_t materialData = 0;
		uint32_t flags = 0;
		Material() = default;
	};

	struct BlockGPU {
		uint32_t materialIDs[6] = { 0 };
		BlockGPU() = default;
	};

	PointerList<Material, 140> materialData;

	namespace Renderer3D {
		const int OctreeCount = GetTreeCount(3);
		namespace {
			uint32_t vertexSize = 0;
			uint32_t indexSize = 0;
			uint32_t meshSize = 0;
			glm::ivec2 renderSize = glm::ivec2(0);
			SceneData sceneData;
		}

		enum class BloomMode
		{
			Prefilter,
			Downsample,
			UpsampleFirst,
			Upsample
		};

		wc::Buffer m_MaterialsBuffer;
		wc::Buffer m_BlockDataBuffer;
		wc::Buffer m_VoxelBuffer;

		wc::ComputeShader shader;
		wc::DescriptorSet mainDescriptorSet;
		wc::Buffer vertexBuffer;
		wc::Buffer indexBuffer;

		bool m_UpdateModels = true;

		wc::Buffer m_DrawCommandBuffer;
		wc::CPUBufferManager<DrawCommand> m_DrawCommands;

		wc::Buffer BVHBuffer;
		wc::Buffer ChunkNodeBuffer;

		wc::Buffer m_LightBuffer;
		wc::CPUBufferManager<Light> lights;
		uint32_t maxLights = chunkVolume;

		// Composite stuff
		wc::Image scrImage;
		wc::ImageView screenImageView;
		wc::Sampler screenSampler;


		wc::ComputeShader compositeShader;
		wc::DescriptorSet compositeSet;
		wc::Image finalImage;

		wc::Texture m_RenderTexture; // @TODO: This is a copy to final image and screenSampler so try to remove it
		// Bloom
		wc::Sampler bloomSampler;

		wc::ComputeShader bloomShader;
		std::vector<VkDescriptorSet> bloomSets;

		uint32_t m_BloomComputeWorkGroupSize = 4; // @TODO: REMOVE!!!
		uint32_t m_BloomMipLevels = 1;

		struct BloomImage {
			std::vector<ImageView> imageViews;
			Image image;

			void Create(uint32_t width, uint32_t height, uint32_t mipLevels) {
				VkImageCreateInfo imgInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };

				imgInfo.imageType = VK_IMAGE_TYPE_2D;

				imgInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;

				imgInfo.extent.width = width;
				imgInfo.extent.height = height;
				imgInfo.extent.depth = 1;

				imgInfo.mipLevels = mipLevels;
				imgInfo.arrayLayers = 1;
				imgInfo.samples = VK_SAMPLE_COUNT_1_BIT;
				imgInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
				imgInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;

				image.Create(imgInfo);

				UploadContext::immediate_submit([&](VkCommandBuffer cmd) {
					VkImageSubresourceRange range;
				range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				range.baseArrayLayer = 0;
				range.baseMipLevel = 0;
				range.layerCount = 1;
				range.levelCount = imgInfo.mipLevels;
				image.setLayout(cmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, range);
					});

				{
					VkImageViewCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
					createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
					createInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
					createInfo.flags = 0;
					createInfo.image = image;
					createInfo.subresourceRange.layerCount = 1;
					createInfo.subresourceRange.levelCount = imgInfo.mipLevels;
					createInfo.subresourceRange.baseMipLevel = 0;
					createInfo.subresourceRange.baseArrayLayer = 0;
					createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

					{ // Creating the first image view
						ImageView& imageView = imageViews.emplace_back();
						imageView.Create(createInfo);
					}

					// Create The rest
					createInfo.subresourceRange.levelCount = 1;
					for (uint32_t i = 1; i < imgInfo.mipLevels; i++)
					{
						createInfo.subresourceRange.baseMipLevel = i;
						ImageView& imageView = imageViews.emplace_back();
						imageView.Create(createInfo);
					}
				}
			}

			void Destroy() {
				for (auto& view : imageViews) view.Destroy();
				image.Destroy();
			}
		} m_BloomBuffers[3];

		struct BloomBufferSettings {
			glm::vec4 Params = glm::vec4(1.f); // (x) threshold, (y) threshold - knee, (z) knee * 2, (w) 0.25 / knee
			float LOD = 0.f;
			int Mode = (int)BloomMode::Prefilter;
		};

		void GenerateBloomDescriptor(const ImageView& outputView, const ImageView& bloomView) {
			DescriptorSet& descriptor = bloomSets.emplace_back();
			descriptorAllocator.allocate(descriptor, bloomShader.getDescriptorLayout());

			{
				DescriptorWriter writer;
				writer.dstSet = descriptor;
				writer.write_image(0, GetDescriptorData(bloomSampler, outputView, VK_IMAGE_LAYOUT_GENERAL), VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
				writer.Update();
			}
			{
				wc::DescriptorWriter writer;
				writer.dstSet = descriptor;
				writer.write_image(1, GetDescriptorData(bloomSampler, bloomView, VK_IMAGE_LAYOUT_GENERAL), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
				writer.Update();
			}
			{
				wc::DescriptorWriter writer;
				writer.dstSet = descriptor;
				writer.write_image(2, GetDescriptorData(bloomSampler, m_BloomBuffers[2].imageViews[0], VK_IMAGE_LAYOUT_GENERAL), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
				writer.Update();
			}
		}

		void AllocateMesh(uint32_t vertexCount, uint32_t indexCount) {
			vertexSize += vertexCount;
			indexSize += indexCount;
			meshSize++;
		}

		void Draw(const DrawCommand& cmd, const glm::vec3& transform) {
			DrawCommand newCmd = cmd;

			newCmd.transform = transform;
			m_UpdateModels = true;

			m_DrawCommands.Add(newCmd);
		}

		void removeModel(const DrawCommand& cmd, const glm::vec3& transform) {
			for (uint32_t i = 0; i < m_DrawCommands.GetCounter(); i++)
				if (m_DrawCommands[i].transform == transform && m_DrawCommands[i].baseVertex == cmd.baseVertex && m_DrawCommands[i].bvhID == cmd.bvhID && m_DrawCommands[i].count == cmd.count && m_DrawCommands[i].firstIndex == cmd.firstIndex) {
					m_DrawCommands.Remove(i);

					break;
				}
		}

		void CreateScreen(glm::ivec2 size) {
			renderSize = size;
			{
				VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
				imageInfo.imageType = VK_IMAGE_TYPE_2D;
				imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
				imageInfo.extent.width = renderSize.x;
				imageInfo.extent.height = renderSize.y;
				imageInfo.extent.depth = 1;
				imageInfo.mipLevels = 1;
				imageInfo.arrayLayers = 1;
				imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
				imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
				imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
				scrImage.Create(imageInfo);
				scrImage.SetName("scrImage");

				VkImageViewCreateInfo imageView = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
				imageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
				imageView.format = imageInfo.format;
				imageView.subresourceRange = {};
				imageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageView.subresourceRange.layerCount = 1;
				imageView.subresourceRange.levelCount = 1;
				imageView.image = scrImage;
				screenImageView.Create(imageView);
				screenImageView.SetName("screenImageView");
			}

			// Create image for this attachment
			{
				//allocate and create the image
				VkImageCreateInfo info = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };

				info.imageType = VK_IMAGE_TYPE_2D;

				info.format = VK_FORMAT_R32G32B32A32_SFLOAT;

				info.extent.width = static_cast<uint32_t>(renderSize.x);
				info.extent.height = static_cast<uint32_t>(renderSize.y);
				info.extent.depth = 1;

				info.mipLevels = 1;
				info.arrayLayers = 1;
				info.samples = VK_SAMPLE_COUNT_1_BIT;
				info.tiling = VK_IMAGE_TILING_OPTIMAL;
				info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

				finalImage.Create(info);

				finalImage.layout = VK_IMAGE_LAYOUT_GENERAL;
				scrImage.layout = VK_IMAGE_LAYOUT_GENERAL;
				UploadContext::immediate_submit([&](VkCommandBuffer cmd) {
					finalImage.setLayout(cmd, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
				scrImage.setLayout(cmd, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
					});
			}

			glm::ivec2 bloomTexSize = renderSize / 2;
			bloomTexSize += glm::ivec2(m_BloomComputeWorkGroupSize - bloomTexSize.x % m_BloomComputeWorkGroupSize, m_BloomComputeWorkGroupSize - bloomTexSize.y % m_BloomComputeWorkGroupSize);
			m_BloomMipLevels = scrImage.GetMipLevelCount() - 4;

			SamplerCreateInfo sampler;

			sampler.magFilter = Filter::LINEAR;
			sampler.minFilter = Filter::LINEAR;
			sampler.mipmapMode = SamplerMipmapMode::LINEAR;
			sampler.addressModeU = SamplerAddressMode::CLAMP_TO_EDGE;
			sampler.addressModeV = SamplerAddressMode::CLAMP_TO_EDGE;
			sampler.addressModeW = SamplerAddressMode::CLAMP_TO_EDGE;
			sampler.minLod = 0.f;
			sampler.maxLod = float(m_BloomMipLevels);

			screenSampler.Create(sampler);
			bloomSampler.Create(sampler);


			for (int i = 0; i < 3; i++) {
				m_BloomBuffers[i].Create(bloomTexSize.x, bloomTexSize.y, m_BloomMipLevels);
				m_BloomBuffers[i].image.SetName(std::format("m_BloomBuffers[{}]", i));
			}

			m_RenderTexture.Create(finalImage, screenSampler, screenImageView);
		}

		void Build(glm::ivec2 size, uint32_t materialSize, uint32_t chunksSize) {

			m_MaterialsBuffer.Create(materialData.byte_size());

			m_VoxelBuffer.Create(sizeof(BlockID) * chunkVolume * chunksSize);

			vertexBuffer.Create(sizeof(Vertex) * vertexSize);
			indexBuffer.Create(sizeof(uint32_t) * indexSize);
			m_LightBuffer.Create(maxLights * sizeof(Light));
			lights.Create(maxLights * sizeof(Light));
			lights.Map();

			vertexBuffer.SetName("vertexBuffer");
			indexBuffer.SetName("indexBuffer");

			m_DrawCommandBuffer.Create(sizeof(DrawCommand) * chunkVolume);
			m_DrawCommandBuffer.SetName("m_DrawCommandBuffer");

			m_BlockDataBuffer.Create(sizeof(BlockGPU) * materialSize);

			m_DrawCommands.Create(sizeof(DrawCommand) * chunkVolume);
			m_DrawCommands.Map();

			BVHBuffer.Create(sizeof(Node) * meshSize);
			BVHBuffer.SetName("BVHBuffer");

			ChunkNodeBuffer.Create(sizeof(ChunkNode) * (chunksSize + OctreeCount));
			ChunkNodeBuffer.SetName("ChunkNodeBuffer");

			wc::ComputeShaderCreateInfo createInfo;
			createInfo.path = GetAssetPath() + "/shaders/rayTracingShader.comp";
			createInfo.infoPath = GetAssetPath() + "/shaders/rayTracingShader.si";
			createInfo.cachePath = wc::GetCachedAssetPath() + "/shaders/rayTracingShader.bin";

			VkDescriptorBindingFlags flags[11];
			memset(flags, 0, sizeof(VkDescriptorBindingFlags) * (std::size(flags) - 1));
			flags[std::size(flags) - 1] = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

			uint32_t counts[1];
			counts[0] = AssetManager::m_Textures.size(); // Set 0 has a variable count descriptor with a maximum of 32 elements

			VkDescriptorSetVariableDescriptorCountAllocateInfo set_counts = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO };
			set_counts.descriptorSetCount = 1;
			set_counts.pDescriptorCounts = counts;
			createInfo.bindingFlags = flags;
			createInfo.bindingFlagCount = std::size(flags);
			createInfo.dynamicDescriptorCount = counts[0];

			shader.Create(createInfo);
			shader.SetName("main");

			bloomShader.Create(GetAssetPath() + "/shaders/bloomShader.comp", GetAssetPath() + "/shaders/bloomShader.si");
			compositeShader.Create(GetAssetPath() + "/shaders/composite.comp", GetAssetPath() + "/shaders/composite.si");
			
			wc::descriptorAllocator.allocate(mainDescriptorSet, shader.getDescriptorLayout(), &set_counts, 1);
			wc::descriptorAllocator.allocate(compositeSet, compositeShader.getDescriptorLayout());

			

			CreateScreen(size);
			{
				wc::DescriptorWriter writer;
				writer.dstSet = mainDescriptorSet;
				writer.write_image(9, GetDescriptorData(screenSampler, screenImageView, scrImage), VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

				std::vector<VkDescriptorImageInfo> infos;
				for (auto& image : AssetManager::m_Textures) {
					VkDescriptorImageInfo imageInfo;
					imageInfo.sampler = image.GetSampler();
					imageInfo.imageView = image.GetView();
					imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

					infos.push_back(imageInfo);
				}

				writer.write_images(10, infos, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
				writer.Update();
			}


			{
				GenerateBloomDescriptor(m_BloomBuffers[0].imageViews[0], screenImageView);

				for (uint32_t currentMip = 1; currentMip < m_BloomMipLevels; currentMip++) {
					// Ping 
					GenerateBloomDescriptor(m_BloomBuffers[1].imageViews[currentMip], m_BloomBuffers[0].imageViews[0]);

					// Pong 
					GenerateBloomDescriptor(m_BloomBuffers[0].imageViews[currentMip], m_BloomBuffers[1].imageViews[0]);
				}

				// First Upsample
				GenerateBloomDescriptor(m_BloomBuffers[2].imageViews[m_BloomMipLevels - 1], m_BloomBuffers[0].imageViews[0]);

				for (int currentMip = m_BloomMipLevels - 2; currentMip >= 0; currentMip--)
					GenerateBloomDescriptor(m_BloomBuffers[2].imageViews[currentMip], m_BloomBuffers[0].imageViews[0]);
			}
			{
				wc::DescriptorWriter writer;
				writer.dstSet = compositeSet;
				writer.write_image(0, GetDescriptorData(screenSampler, screenImageView, finalImage), VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
					.write_image(1, GetDescriptorData(screenSampler, screenImageView, scrImage), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
					.write_image(2, GetDescriptorData(bloomSampler, m_BloomBuffers[2].imageViews[0], VK_IMAGE_LAYOUT_GENERAL), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
					.Update();
			}

			
			wc::DescriptorWriter2<9> writer;
			writer.dstSet = mainDescriptorSet;
			writer.write_buffer(0, m_LightBuffer.GetDescriptorInfo(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			writer.write_buffer(1, vertexBuffer.GetDescriptorInfo(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			writer.write_buffer(2, indexBuffer.GetDescriptorInfo(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			writer.write_buffer(3, m_DrawCommandBuffer.GetDescriptorInfo(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			writer.write_buffer(4, BVHBuffer.GetDescriptorInfo(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			writer.write_buffer(5, ChunkNodeBuffer.GetDescriptorInfo(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			writer.write_buffer(6, m_MaterialsBuffer.GetDescriptorInfo(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			writer.write_buffer(7, m_VoxelBuffer.GetDescriptorInfo(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			writer.write_buffer(8, m_BlockDataBuffer.GetDescriptorInfo(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			writer.Update();			
		}

		void UploadModels(const std::vector<Vertex>& vertexData, const std::vector<uint32_t>& indexData, const std::vector<Node>& bvhData) {
			vertexBuffer.SetData(vertexData.data(), sizeof(Vertex) * vertexData.size());
			indexBuffer.SetData(indexData.data(), sizeof(uint32_t) * indexData.size());
			BVHBuffer.SetData(bvhData.data(), sizeof(Node) * bvhData.size());
		}

		void UpdateCamera(const Camera& camera) {
			sceneData.cameraPos = camera.Position;
			sceneData.lower_left_corner = camera.lower_left_corner;
			sceneData.vertical = camera.vertical;
			sceneData.horizontal = camera.horizontal;
			sceneData.numLights = lights.GetCounter();
			sceneData.bvhCounter = m_DrawCommands.GetCounter();
		}

		// LIGHT MANAGING (deprecated)
		uint32_t addLight(const glm::vec3& position, uint32_t color) {
			uint32_t light = lights.GetCounter();
			if (light <= maxLights)
				lights.Add(Light(position, color));

			return light;
		}

		void removeLight(const glm::vec3& position) {
			for (uint32_t i = 0; i < lights.GetCounter(); i++)
				if (lights[i].vector == position) {
					lights.Remove(i);

					break;
				}
		}		

		void Render() {
			if (true) { // if (updateLights)

				uint32_t size = Renderer3D::lights.GetCounter() ? Renderer3D::lights.GetCounter() : 1;
				Renderer3D::lights.Unmap();
				Renderer3D::m_LightBuffer.SetData(Renderer3D::lights.GetBuffer(), size * sizeof(Light));
				Renderer3D::lights.Map();
			}


			if (m_UpdateModels) {
				m_UpdateModels = false;

				size_t size = Renderer3D::m_DrawCommands.GetCounter() ? Renderer3D::m_DrawCommands.GetCounter() : 1;
				Renderer3D::m_DrawCommands.Unmap();
				Renderer3D::m_DrawCommandBuffer.SetData(Renderer3D::m_DrawCommands.GetBuffer(), sizeof(DrawCommand) * size);
				Renderer3D::m_DrawCommands.Map();
			}

			wc::CommandBuffer& cmd = RendererContext::computeCommandBuffer;
			cmd.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

			cmd.BindDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, 0, shader.getPipelineLayout(), mainDescriptorSet);
			shader.Bind(cmd);
			cmd.PushConstants(shader.getPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, sizeof(SceneData), &sceneData);
			cmd.Dispatch(glm::ceil((glm::vec2)renderSize / glm::vec2(m_BloomComputeWorkGroupSize)));
			scrImage.insertMemoryBarrier(cmd, VK_IMAGE_ASPECT_COLOR_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			if (Settings::bloomEnable) {
				cmd.BindPipeline(bloomShader.getPipeline());
				uint32_t counter = 0;

				BloomBufferSettings settings;
				settings.Params = glm::vec4(Settings::BloomThreshold, Settings::BloomThreshold - Settings::BloomKnee, Settings::BloomKnee * 2.f, 0.25f / Settings::BloomKnee);
				cmd.PushConstants(bloomShader.getPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, sizeof(settings), &settings);
				cmd.BindDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, 0, bloomShader.getPipelineLayout(), bloomSets[counter]);
				counter++;
				cmd.Dispatch(glm::ceil(glm::vec2(m_BloomBuffers[0].image.GetSize()) / glm::vec2(m_BloomComputeWorkGroupSize)));

				settings.Mode = (int)BloomMode::Downsample;
				for (uint32_t currentMip = 1; currentMip < m_BloomMipLevels; currentMip++)
				{
					glm::vec2 dispatchSize = glm::ceil((glm::vec2)m_BloomBuffers[0].image.GetMipSize(currentMip) / glm::vec2(m_BloomComputeWorkGroupSize));

					// Ping 
					settings.LOD = float(currentMip - 1);
					cmd.PushConstants(bloomShader.getPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, sizeof(settings), &settings);

					cmd.BindDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, 0, bloomShader.getPipelineLayout(), bloomSets[counter]);
					counter++;
					cmd.Dispatch(dispatchSize);

					// Pong 
					settings.LOD = float(currentMip);
					cmd.PushConstants(bloomShader.getPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, sizeof(settings), &settings);

					cmd.BindDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, 0, bloomShader.getPipelineLayout(), bloomSets[counter]);
					counter++;
					cmd.Dispatch(dispatchSize);
				}

				// First Upsample		
				settings.LOD = float(m_BloomMipLevels - 2);
				settings.Mode = (int)BloomMode::UpsampleFirst;
				cmd.PushConstants(bloomShader.getPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, sizeof(settings), &settings);

				cmd.BindDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, 0, bloomShader.getPipelineLayout(), bloomSets[counter]);
				counter++;

				cmd.Dispatch(glm::ceil((glm::vec2)m_BloomBuffers[2].image.GetMipSize(m_BloomMipLevels - 1) / glm::vec2(m_BloomComputeWorkGroupSize)));

				settings.Mode = (int)BloomMode::Upsample;
				for (int currentMip = m_BloomMipLevels - 2; currentMip >= 0; currentMip--)
				{
					settings.LOD = float(currentMip);
					cmd.PushConstants(bloomShader.getPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, sizeof(settings), &settings);

					cmd.BindDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, 0, bloomShader.getPipelineLayout(), bloomSets[counter]);
					counter++;

					cmd.Dispatch(glm::ceil((glm::vec2)m_BloomBuffers[2].image.GetMipSize(currentMip) / glm::vec2(m_BloomComputeWorkGroupSize)));
				}

			}

			cmd.BindDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, 0, compositeShader.getPipelineLayout(), compositeSet);
			compositeShader.Bind(cmd);
			cmd.Dispatch(glm::ceil((glm::vec2)renderSize / glm::vec2(m_BloomComputeWorkGroupSize)));

			cmd.End();


			VkSubmitInfo submit = { VK_STRUCTURE_TYPE_SUBMIT_INFO };

			submit.commandBufferCount = 1;
			submit.pCommandBuffers = cmd.GetPointer();

			//submit.pWaitSemaphores = RendererContext::computeSemaphore.GetPointer();
			//submit.waitSemaphoreCount = 1;

			VkPipelineStageFlags computeWaitStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

			submit.pWaitDstStageMask = &computeWaitStage;

			VulkanContext::computeQueue.Submit(submit, RendererContext::computeFence);
			RendererContext::computeFence.Wait();
			RendererContext::computeFence.Reset();
		}

		void DestroyScreen() {
			bloomSampler.Destroy();
			for (int i = 0; i < 3; i++) m_BloomBuffers[i].Destroy();
			screenSampler.Destroy();

			scrImage.Destroy();
			screenImageView.Destroy();

			finalImage.Destroy();
		}

		void Destroy() {
			//shader.SaveCache();
			shader.Destroy();
			compositeShader.Destroy();
			bloomShader.Destroy();
			vertexBuffer.Destroy();
			indexBuffer.Destroy();
			BVHBuffer.Destroy();
			ChunkNodeBuffer.Destroy();

			m_MaterialsBuffer.Destroy();
			m_BlockDataBuffer.Destroy();

			m_VoxelBuffer.Destroy();

			m_LightBuffer.Destroy();
			lights.Unmap(); lights.Destroy();

			m_DrawCommandBuffer.Destroy();
			m_DrawCommands.Unmap(); m_DrawCommands.Destroy();

			DestroyScreen();
		}		
	}

	struct Mesh {
		DrawCommand cmd = {};

		Mesh() = default;

		void Load(const std::string& path, uint32_t materialID, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::vector<Node>& bvhData) {
			Assimp::Importer importer;
			const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices | aiProcess_GenNormals | aiProcess_FlipUVs);

			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			{
				WC_ERROR(importer.GetErrorString());
				return;
			}
			uint32_t offset = 0;
			processNode(scene->mRootNode, *scene, offset, materialID, vertices, indices);

			uint32_t totalVertices = 0;
			uint32_t totalIndices = 0;
			GetMeshSize(scene->mRootNode, *scene, totalIndices, totalVertices);
			uint32_t vertexOffset = Renderer3D::vertexSize;
			uint32_t indexOffset = Renderer3D::indexSize;

			if (false) {
				using Vec3 = bvh::v2::Vec<float, 3>;
				using BBox = bvh::v2::BBox<float, 3>;
				using Tri = bvh::v2::Tri<float, 3>;
				using Nodev2 = bvh::v2::Node<float, 3>;
				using Bvh = bvh::v2::Bvh<Nodev2>;
			
			
				// This is the original data, which may come in some other data type/structure.
				std::vector<Tri> tris;

				for (uint32_t i = indexOffset; i < indices.size(); i += 3) {
					Tri tri;
					glm::vec3 p0 = vertices[indices[i] + vertexOffset + 0].Position;
					glm::vec3 p1 = vertices[indices[i] + vertexOffset + 1].Position;
					glm::vec3 p2 = vertices[indices[i] + vertexOffset + 2].Position;

					tri.p0 = Vec3(p0.x, p0.y, p0.z);
					tri.p1 = Vec3(p1.x, p1.y, p1.z);
					tri.p2 = Vec3(p2.x, p2.y, p2.z);
					tris.push_back(tri);
				}				

				bvh::v2::ThreadPool thread_pool;
				bvh::v2::ParallelExecutor executor(thread_pool);

				// Get triangle centers and bounding boxes (required for BVH builder)
				std::vector<BBox> bboxes(tris.size());
				std::vector<Vec3> centers(tris.size());
				executor.for_each(0, tris.size(), [&](size_t begin, size_t end) {
					for (size_t i = begin; i < end; ++i) {
						bboxes[i] = tris[i].get_bbox();
						centers[i] = tris[i].get_center();
					}
					});

				typename bvh::v2::DefaultBuilder<Nodev2>::Config config;
				config.quality = bvh::v2::DefaultBuilder<Nodev2>::Quality::High;
				auto bvh = bvh::v2::DefaultBuilder<Nodev2>::build(thread_pool, bboxes, centers, config);
				WC_INFO(bvh.nodes.size());
			}

			cmd.baseVertex = vertexOffset;
			cmd.firstIndex = indexOffset;
			cmd.count = totalIndices;

			Renderer3D::AllocateMesh(totalVertices, totalIndices);

			vertexOffset = cmd.baseVertex - vertexOffset;

			Node aabb;
			aabb.start = glm::vec4(vertices[vertexOffset].Position, 1.f);
			aabb.end = glm::vec4(vertices[vertexOffset].Position, 1.f);

			for (uint32_t i = vertexOffset + 1; i < vertices.size(); i++) {
				aabb.start = glm::max(aabb.start, vertices[i].Position);
				aabb.end = glm::min(aabb.end, vertices[i].Position);
			}
			cmd.bvhID = bvhData.size();
			bvhData.push_back(aabb);
			importer.FreeScene();
		}

		void processNode(const aiNode* node, const aiScene& scene, uint32_t& offset, uint32_t materialID, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
			// process each mesh located at the current node		
			// the node object only contains indices to index the actual objects in the scene. 
			// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
			for (uint32_t m = 0; m < node->mNumMeshes; m++) {
				auto& mesh = *scene.mMeshes[node->mMeshes[m]];
				for (uint32_t i = 0; i < mesh.mNumVertices; i++)
				{
					Vertex vertex;
					vertex.Position = AssimpGLMHelpers::GetGLMVec(mesh.mVertices[i]) + glm::vec3(0.5f, 0.f, 0.5f);
					vertex.Normal = -AssimpGLMHelpers::GetGLMVec(mesh.mNormals[i]);
					vertex.materialID = materialID;

					if (mesh.mTextureCoords[0])
						vertex.TexCoords = glm::vec3(mesh.mTextureCoords[0][i].x, mesh.mTextureCoords[0][i].y, 0.f);

					vertices.push_back(vertex);
				}

				for (uint32_t i = 0; i < mesh.mNumFaces; i++)
				{
					aiFace& face = mesh.mFaces[i];
					for (uint32_t j = 0; j < face.mNumIndices; j++)
						indices.push_back(face.mIndices[j] + offset);
				}

				offset += mesh.mNumVertices;
			}

			// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
			for (uint32_t i = 0; i < node->mNumChildren; i++)
				processNode(node->mChildren[i], scene, offset, materialID, vertices, indices);
		}

		void GetMeshSize(const aiNode* node, const aiScene& scene, uint32_t& totalIndices, uint32_t& totalVertices) {
			// process each mesh located at the current node		
			// the node object only contains indices to index the actual objects in the scene. 
			// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
			for (uint32_t j = 0; j < node->mNumMeshes; j++) {
				auto& mesh = scene.mMeshes[node->mMeshes[j]];
				totalVertices += mesh->mNumVertices;
				for (uint32_t i = 0; i < mesh->mNumFaces; i++)
					totalIndices += mesh->mFaces[i].mNumIndices;

			}
			// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
			for (uint32_t i = 0; i < node->mNumChildren; i++)
				GetMeshSize(node->mChildren[i], scene, totalIndices, totalVertices);
		}
	};

	List<Mesh, 10> blockMeshes;
}