#pragma once

#include <wc/Shader.h>
#include <wc/vk/Buffer.h>
#include <wc/vk/Images.h>
#include <wc/Framebuffer.h>
#include <wc/Utils/DeletionQueue.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <wc/Math/AssimpGLMHelpers.h>

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
#include "../Globals.h"
#include <wc/Math/Camera.h>
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
		glm::vec3 transform;
		uint32_t bvhID;
	};

	struct Node {
		glm::vec3 min;
		uint32_t first;
		glm::vec3 max;
		uint32_t primCount;
	};

	const uint32_t IS_LEAF_BIT =  0b00000001u;
	const uint32_t IS_EMPTY_BIT = 0b00000010u;

	struct ChunkNode {
		ChunkNode() = default;

		glm::vec3 min = glm::vec3(0.f);
		uint32_t flags = 0;
		glm::vec3 max = glm::vec3(0.f);
		uint32_t parentID = 0; // not really used in shader just for convinience
		uint32_t children[8] = { 0 };
	};

	struct Light {
		glm::vec3 vector;
		float intensity;
		glm::vec3 color;
		float radius;
	};

	enum ShadingModel {
		Unlit,
		DefaultLit,
		ClearCoat,
		Anisotropic,
		Subsurface,
		Cloth
	};

	struct BufferMaterial {
		BufferMaterial() = default;

		uint32_t albedo = 0;
		float metallic = 0.f;
		float roughness = 1.f;
		float reflectance = 0.5f;
		float clearCoat = 1.f;
		float clearCoatRoughness = 0.f;
		float anisotropy = 0.f;
		//glm::vec3 anisotropyDirection = glm::vec3(0.f);
		float ior = 1.f;
		float emissive = 0.f;
	};

	struct BlockGPU {
		uint32_t materialIDs[6] = { 0 };
		BlockGPU() = default;
	};

	PointerList<BufferMaterial, 140> materialData;
	enum class BloomMode
	{
		Prefilter,
		Downsample,
		UpsampleFirst,
		Upsample
	};

	const int OctreeCount = GetTreeCount(4);

	inline uint32_t s_VertexSize;
	inline uint32_t s_IndexSize;
	inline uint32_t s_NodeSize;
	
	inline Buffer s_MaterialsBuffer;
	inline Buffer s_BlockDataBuffer;
	inline Buffer s_VoxelBuffer;
	inline Buffer s_VertexBuffer;
	inline Buffer s_IndexBuffer;
	inline Buffer s_BVHBuffer;

	inline ComputeShader shader;
	inline ComputeShader compositeShader;
	inline ComputeShader bloomShader;
	class Renderer3D {
	private:

		glm::ivec2 m_RenderSize = glm::ivec2(0);
		float m_AspectRatio = 16.f / 9.f;

		struct SceneData {
			glm::vec3 cameraPos = glm::vec3(0.f);
			uint32_t numLights = 0;
			glm::vec3 lower_left_corner = glm::vec3(0.f);
			uint32_t bvhCounter = 0;
			glm::vec3 horizontal = glm::vec3(0.f);
			uint32_t MaxBounceCount = 2;
			glm::vec3 vertical = glm::vec3(0.f);
			uint32_t NumRaysPerPixel = 2;
			uint32_t Frame = 0;
		} m_SceneData;		

		DescriptorSet m_MainDescriptorSet;

		bool m_UpdateModels = true;

		Buffer m_DrawCommandBuffer;
		CPUBufferManager<DrawCommand> m_DrawCommands;

		Buffer m_ChunkNodeBuffer;

		Buffer m_LightBuffer;

		// Composite stuff
		Image m_OutputImage;
		Image m_FinalImage;

		ImageView m_OutputImageView;
		ImageView m_FinalImageView;

		Sampler screenSampler;


		DescriptorSet compositeSet;

		ImTextureID m_RenderTextureID;
		// Bloom
		Sampler m_BloomSampler;

		std::vector<VkDescriptorSet> m_BloomSets;

		uint32_t m_BloomComputeWorkGroupSize = 4; // @TODO: REMOVE!!!
		uint32_t m_BloomMipLevels = 1;

		struct BloomImage {
			std::vector<ImageView> imageViews;
			Image image;

			void Create(uint32_t width, uint32_t height, uint32_t mipLevels) {
				ImageCreateInfo imgInfo;

				imgInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;

				imgInfo.width = width;
				imgInfo.height = height;

				imgInfo.mipLevels = mipLevels;
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
			DescriptorSet& descriptor = m_BloomSets.emplace_back();
			descriptorAllocator.allocate(descriptor, bloomShader.getDescriptorLayout());

			{
				DescriptorWriter writer;
				writer.dstSet = descriptor;
				writer.write_image(0, GetDescriptorData(m_BloomSampler, outputView, VK_IMAGE_LAYOUT_GENERAL), VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
				writer.Update();
			}
			{
				DescriptorWriter writer;
				writer.dstSet = descriptor;
				writer.write_image(1, GetDescriptorData(m_BloomSampler, bloomView, VK_IMAGE_LAYOUT_GENERAL), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
				writer.Update();
			}
			{
				DescriptorWriter writer;
				writer.dstSet = descriptor;
				writer.write_image(2, GetDescriptorData(m_BloomSampler, m_BloomBuffers[2].imageViews[0], VK_IMAGE_LAYOUT_GENERAL), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
				writer.Update();
			}
		}
	public:
		struct Vertex {
			glm::vec3 Position = { 0,0,0 };
			uint32_t _pad1 = 0;
			glm::vec3 Normal = { 0,0,0 };
			uint32_t _pad2 = 0;
			glm::vec3 Tangent = { 0,0,0 };
			uint32_t _pad3 = 0;
			glm::vec3 Bitangent = { 0,0,0 };
			uint32_t _pad4 = 0;
			glm::vec2 TexCoords = { 0,0 };
			uint32_t materialID = 0;
			uint32_t _pad = { 0 };
			Vertex() {}
			Vertex(const glm::vec3& pos, glm::vec2 texCoord, const glm::vec3& normal, uint32_t mat) : Position(pos), Normal(normal), Tangent(0), Bitangent(0), materialID(mat), TexCoords(texCoord) {}
		};

		CPUBufferManager<Light> m_Lights;
		Camera camera;
		static void AllocateMesh(uint32_t vertexCount, uint32_t indexCount, uint32_t nodeCount) {
			s_VertexSize += vertexCount;
			s_IndexSize += indexCount;
			s_NodeSize += nodeCount;
		}

		static auto GetVertexSize() { return s_VertexSize; }
		static auto GetIndexSize() { return s_IndexSize; }
		static auto& GetVoxelBuffer() { return s_VoxelBuffer; }
		static auto& GetBlockDataBuffer() { return s_BlockDataBuffer; }
		static auto& GetMaterialsBuffer() { return s_MaterialsBuffer; }

		auto GetAspectRatio() const { return m_AspectRatio; }
		auto GetOutputImage() { return m_OutputImage; }
		auto GetFinalImage() { return m_FinalImage; }
		auto GetRenderTexture() { return m_RenderTextureID; }
		auto& GetChunkNodeBuffer() { return m_ChunkNodeBuffer; }
		auto GetScreenSize() { return m_RenderSize; }

		void Draw(const DrawCommand& cmd, const glm::vec3& transform) {
			DrawCommand newCmd = cmd;

			newCmd.transform = transform;
			m_UpdateModels = true;

			m_DrawCommands.Add(newCmd);
		}

		void removeModel(const DrawCommand& cmd, const glm::vec3& transform) {
			for (uint32_t i = 0; i < m_DrawCommands.GetCounter(); i++)
				if (m_DrawCommands[i].transform == transform && m_DrawCommands[i].bvhID == cmd.bvhID) {
					m_DrawCommands.Remove(i);

					return;
				}
		}

		void CreateScreen(glm::ivec2 size) {
			m_RenderSize = size;
			m_AspectRatio = (float)m_RenderSize.x / (float)m_RenderSize.y;
			{
				ImageCreateInfo imageInfo;
				imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
				imageInfo.width = m_RenderSize.x;
				imageInfo.height = m_RenderSize.y;
				imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
				m_OutputImage.Create(imageInfo);
				m_FinalImage.Create(imageInfo);

				m_OutputImage.SetName("outputImage");
				m_FinalImage.SetName("finalImage");

				VkImageViewCreateInfo imageView = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
				imageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
				imageView.format = imageInfo.format;
				imageView.subresourceRange = {};
				imageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageView.subresourceRange.layerCount = 1;
				imageView.subresourceRange.levelCount = 1;

				imageView.image = m_OutputImage;
				m_OutputImageView.Create(imageView);

				imageView.image = m_FinalImage;
				m_FinalImageView.Create(imageView);

				m_OutputImageView.SetName("outputImageView");
				m_FinalImage.SetName("FinalImageView");

				UploadContext::immediate_submit([&](VkCommandBuffer cmd) {
					m_OutputImage.setLayout(cmd, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
					m_FinalImage.setLayout(cmd, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
				});
			}

			glm::ivec2 bloomTexSize = m_RenderSize / 2;
			bloomTexSize += glm::ivec2(m_BloomComputeWorkGroupSize - bloomTexSize.x % m_BloomComputeWorkGroupSize, m_BloomComputeWorkGroupSize - bloomTexSize.y % m_BloomComputeWorkGroupSize);
			m_BloomMipLevels = m_OutputImage.GetMipLevelCount() - 4;

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
			m_BloomSampler.Create(sampler);


			for (int i = 0; i < 3; i++) {
				m_BloomBuffers[i].Create(bloomTexSize.x, bloomTexSize.y, m_BloomMipLevels);
				m_BloomBuffers[i].image.SetName(std::format("m_BloomBuffers[{}]", i));
			}

			m_RenderTextureID = ImGui_ImplVulkan_AddTexture(screenSampler, m_FinalImageView, VK_IMAGE_LAYOUT_GENERAL);
		}

		static void Init(const AssetManager& assetManager) {
			ComputeShaderCreateInfo createInfo;
			createInfo.path = GetAssetPath() + "/shaders/rayTracingShader.comp";
			createInfo.infoPath = GetAssetPath() + "/shaders/rayTracingShader.si";
			createInfo.cachePath = GetCachedAssetPath() + "/shaders/rayTracingShader.bin";

			VkDescriptorBindingFlags flags[11];
			memset(flags, 0, sizeof(VkDescriptorBindingFlags) * (std::size(flags) - 1));
			flags[std::size(flags) - 1] = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

			
			createInfo.bindingFlags = flags;
			createInfo.bindingFlagCount = std::size(flags);
			createInfo.dynamicDescriptorCount = assetManager.m_Textures.size();

			shader.Create(createInfo);
			shader.SetName("main");

			bloomShader.Create(GetAssetPath() + "/shaders/bloomShader.comp", GetAssetPath() + "/shaders/bloomShader.si");
			compositeShader.Create(GetAssetPath() + "/shaders/composite.comp", GetAssetPath() + "/shaders/composite.si");
		}

		static void CreateBuffers(uint32_t materialSize, uint32_t chunksSize) {
			s_VertexBuffer.Create(sizeof(Vertex) * s_VertexSize);
			s_IndexBuffer.Create(sizeof(uint32_t) * s_IndexSize);
			s_BVHBuffer.Create(sizeof(Node) * s_NodeSize);

			s_MaterialsBuffer.Create(materialData.byte_size());

			s_VoxelBuffer.Create(sizeof(BlockID) * chunkVolume * chunksSize);
			s_BlockDataBuffer.Create(sizeof(BlockGPU) * materialSize);

			s_VertexBuffer.SetName("Renderer3D::VertexBuffer");
			s_IndexBuffer.SetName("Renderer3D::IndexBuffer");
			s_BVHBuffer.SetName("Renderer3D::BVHBuffer");

			s_MaterialsBuffer.SetName("Renderer3D::MaterialsBuffer");

			s_VoxelBuffer.SetName("Renderer3D::VoxelBuffer"); // TODO: Maybe shouldn't be static
			s_BlockDataBuffer.SetName("Renderer3D::BlockDataBuffer");
		}

		void Create(glm::ivec2 size, const AssetManager& assetManager) {

			m_LightBuffer.Create(sizeof(Light));
			m_Lights.Create(sizeof(Light));
			m_Lights.Map();			

			m_DrawCommandBuffer.Create(sizeof(DrawCommand) * chunkVolume);
			m_DrawCommandBuffer.SetName("m_DrawCommandBuffer");			

			m_DrawCommands.Create(sizeof(DrawCommand) * chunkVolume);
			m_DrawCommands.Map();

			m_ChunkNodeBuffer.Create(sizeof(ChunkNode) * OctreeCount);
			m_ChunkNodeBuffer.SetName("ChunkNodeBuffer");


			uint32_t counts[1];
			counts[0] = assetManager.m_Textures.size(); // Set 0 has a variable count descriptor with a maximum of 32 elements

			VkDescriptorSetVariableDescriptorCountAllocateInfo set_counts = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO };
			set_counts.descriptorSetCount = 1;
			set_counts.pDescriptorCounts = counts;
			descriptorAllocator.allocate(m_MainDescriptorSet, shader.getDescriptorLayout(), &set_counts, 1);
			descriptorAllocator.allocate(compositeSet, compositeShader.getDescriptorLayout());

			

			CreateScreen(size);
			{
				DescriptorWriter writer;
				writer.dstSet = m_MainDescriptorSet;
				writer.write_image(9, GetDescriptorData(screenSampler, m_OutputImageView, VK_IMAGE_LAYOUT_GENERAL), VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

				std::vector<VkDescriptorImageInfo> infos;
				for (auto& image : assetManager.m_Textures) {
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
				GenerateBloomDescriptor(m_BloomBuffers[0].imageViews[0], m_OutputImageView);

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
				DescriptorWriter writer;
				writer.dstSet = compositeSet;
				writer.write_image(0, GetDescriptorData(screenSampler, m_FinalImageView, VK_IMAGE_LAYOUT_GENERAL), VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
					.write_image(1, GetDescriptorData(screenSampler, m_OutputImageView, VK_IMAGE_LAYOUT_GENERAL), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
					.write_image(2, GetDescriptorData(m_BloomSampler, m_BloomBuffers[2].imageViews[0], VK_IMAGE_LAYOUT_GENERAL), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
					.Update();
			}

			
			DescriptorWriter2<9> writer;
			writer.dstSet = m_MainDescriptorSet;
			writer.write_buffer(0, m_LightBuffer.GetDescriptorInfo(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			writer.write_buffer(1, s_VertexBuffer.GetDescriptorInfo(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			writer.write_buffer(2, s_IndexBuffer.GetDescriptorInfo(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			writer.write_buffer(3, m_DrawCommandBuffer.GetDescriptorInfo(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			writer.write_buffer(4, s_BVHBuffer.GetDescriptorInfo(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			writer.write_buffer(5, m_ChunkNodeBuffer.GetDescriptorInfo(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			writer.write_buffer(6, s_MaterialsBuffer.GetDescriptorInfo(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			writer.write_buffer(7, s_VoxelBuffer.GetDescriptorInfo(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			writer.write_buffer(8, s_BlockDataBuffer.GetDescriptorInfo(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			writer.Update();			
		}

		static void UploadModels(const std::vector<Vertex>& vertexData, const std::vector<uint32_t>& indexData, const std::vector<Node>& bvhData) {
			s_VertexBuffer.SetData(vertexData.data(), sizeof(Vertex) * vertexData.size());
			s_IndexBuffer.SetData(indexData.data(), sizeof(uint32_t) * indexData.size());
			s_BVHBuffer.SetData(bvhData.data(), sizeof(Node) * bvhData.size());
		}

		// LIGHT MANAGING (deprecated)
		uint32_t addLight(const glm::vec3& position, const glm::vec3& color, float intensity, float radius) {
			uint32_t light = m_Lights.GetCounter();
			m_Lights.Add({ position, intensity, color, radius});

			return light;
		}

		bool eq(const glm::vec3& v1, const glm::vec3& v2) {
			return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
		}

		void Render() {
			m_Lights.Unmap();
			m_LightBuffer.SetData(m_Lights.GetBuffer(), sizeof(Light));
			m_Lights.Map();
			


			if (m_UpdateModels) {
				m_UpdateModels = false;

				size_t size = m_DrawCommands.GetCounter() ? m_DrawCommands.GetCounter() : 1;
				m_DrawCommands.Unmap();
				m_DrawCommandBuffer.SetData(m_DrawCommands.GetBuffer(), sizeof(DrawCommand) * size);
				m_DrawCommands.Map();
			}

			CommandBuffer& cmd = RendererContext::computeCommandBuffer;
			cmd.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

			cmd.BindDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, 0, shader.getPipelineLayout(), m_MainDescriptorSet);
			shader.Bind(cmd);

			m_SceneData.Frame++;
			if (m_SceneData.cameraPos != camera.Position || m_SceneData.lower_left_corner != camera.lower_left_corner || m_SceneData.vertical != camera.vertical || m_SceneData.horizontal != camera.horizontal) m_SceneData.Frame = 0;

			m_SceneData.cameraPos = camera.Position;
			m_SceneData.lower_left_corner = camera.lower_left_corner;
			m_SceneData.vertical = camera.vertical;
			m_SceneData.horizontal = camera.horizontal;
			m_SceneData.numLights = m_Lights.GetCounter();
			m_SceneData.bvhCounter = m_DrawCommands.GetCounter();
			m_SceneData.MaxBounceCount = Settings::maxBounceCount;
			m_SceneData.NumRaysPerPixel = Settings::raysPerPixel;

			shader.PushConstants(cmd, sizeof(SceneData), &m_SceneData);
			cmd.Dispatch(glm::ceil((glm::vec2)m_RenderSize / glm::vec2(m_BloomComputeWorkGroupSize)));
			m_OutputImage.insertMemoryBarrier(cmd, VK_IMAGE_ASPECT_COLOR_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			if (Settings::bloomEnable) {
				cmd.BindPipeline(bloomShader.getPipeline());
				uint32_t counter = 0;

				BloomBufferSettings settings;
				settings.Params = glm::vec4(Settings::BloomThreshold, Settings::BloomThreshold - Settings::BloomKnee, Settings::BloomKnee * 2.f, 0.25f / Settings::BloomKnee);
				bloomShader.PushConstants(cmd, sizeof(settings), &settings);
				cmd.BindDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, 0, bloomShader.getPipelineLayout(), m_BloomSets[counter]);
				counter++;
				cmd.Dispatch(glm::ceil(glm::vec2(m_BloomBuffers[0].image.GetSize()) / glm::vec2(m_BloomComputeWorkGroupSize)));

				settings.Mode = (int)BloomMode::Downsample;
				for (uint32_t currentMip = 1; currentMip < m_BloomMipLevels; currentMip++)
				{
					glm::vec2 dispatchSize = glm::ceil((glm::vec2)m_BloomBuffers[0].image.GetMipSize(currentMip) / glm::vec2(m_BloomComputeWorkGroupSize));

					// Ping 
					settings.LOD = float(currentMip - 1);
					bloomShader.PushConstants(cmd, sizeof(settings), &settings);

					cmd.BindDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, 0, bloomShader.getPipelineLayout(), m_BloomSets[counter]);
					counter++;
					cmd.Dispatch(dispatchSize);

					// Pong 
					settings.LOD = float(currentMip);
					bloomShader.PushConstants(cmd, sizeof(settings), &settings);

					cmd.BindDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, 0, bloomShader.getPipelineLayout(), m_BloomSets[counter]);
					counter++;
					cmd.Dispatch(dispatchSize);
				}

				// First Upsample		
				settings.LOD = float(m_BloomMipLevels - 2);
				settings.Mode = (int)BloomMode::UpsampleFirst;
				bloomShader.PushConstants(cmd, sizeof(settings), &settings);

				cmd.BindDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, 0, bloomShader.getPipelineLayout(), m_BloomSets[counter]);
				counter++;

				cmd.Dispatch(glm::ceil((glm::vec2)m_BloomBuffers[2].image.GetMipSize(m_BloomMipLevels - 1) / glm::vec2(m_BloomComputeWorkGroupSize)));

				settings.Mode = (int)BloomMode::Upsample;
				for (int currentMip = m_BloomMipLevels - 2; currentMip >= 0; currentMip--)
				{
					settings.LOD = float(currentMip);
					bloomShader.PushConstants(cmd, sizeof(settings), &settings);

					cmd.BindDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, 0, bloomShader.getPipelineLayout(), m_BloomSets[counter]);
					counter++;

					cmd.Dispatch(glm::ceil((glm::vec2)m_BloomBuffers[2].image.GetMipSize(currentMip) / glm::vec2(m_BloomComputeWorkGroupSize)));
				}

			}

			cmd.BindDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, 0, compositeShader.getPipelineLayout(), compositeSet);
			compositeShader.Bind(cmd);
			compositeShader.PushConstants(cmd, sizeof(uint32_t), &Settings::toneMapFunctionID);
			cmd.Dispatch(glm::ceil((glm::vec2)m_RenderSize / glm::vec2(m_BloomComputeWorkGroupSize)));

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
			m_BloomSampler.Destroy();
			for (int i = 0; i < 3; i++) m_BloomBuffers[i].Destroy();
			screenSampler.Destroy();

			m_OutputImage.Destroy();
			m_FinalImage.Destroy();

			m_OutputImageView.Destroy();
			m_FinalImageView.Destroy();
		}

		void Destroy() {
			
			m_ChunkNodeBuffer.Destroy();

			m_LightBuffer.Destroy();
			m_Lights.Unmap(); m_Lights.Destroy();

			m_DrawCommandBuffer.Destroy();
			m_DrawCommands.Unmap(); m_DrawCommands.Destroy();

			DestroyScreen();
		}

		static void Deinit() {
			//shader.SaveCache();
			shader.Destroy();
			compositeShader.Destroy();
			bloomShader.Destroy();

			s_VertexBuffer.Destroy();
			s_IndexBuffer.Destroy();
			s_BVHBuffer.Destroy();

			s_MaterialsBuffer.Destroy();
			s_BlockDataBuffer.Destroy();

			s_VoxelBuffer.Destroy();
		}
	};

	struct Mesh {
		DrawCommand cmd = {};

		Mesh() = default;

		void Load(const std::string& path, uint32_t materialID, std::vector<Renderer3D::Vertex>& vertices, std::vector<uint32_t>& indices, std::vector<Node>& bvhData) {
			Assimp::Importer importer;
			const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices | aiProcess_GenNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

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
			uint32_t vertexOffset = Renderer3D::GetVertexSize();
			uint32_t indexOffset = Renderer3D::GetIndexSize();

#if 0
			using Vec3 = bvh::v2::Vec<float, 3>;
			using BBox = bvh::v2::BBox<float, 3>;
			using Tri = bvh::v2::Tri<float, 3>;
			using Nodev2 = bvh::v2::Node<float, 3>;
			using Bvh = bvh::v2::Bvh<Nodev2>;


			// This is the original data, which may come in some other data type/structure.
			std::vector<Tri> tris;

			for (uint32_t i = indexOffset; i < indices.size(); i += 3) {
				Tri tri;
				glm::vec3 p0 = vertices[indices[i] + 0].Position;
				glm::vec3 p1 = vertices[indices[i] + 1].Position;
				glm::vec3 p2 = vertices[indices[i] + 2].Position;

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

			bvh::v2::DefaultBuilder<Nodev2>::Config config;
			config.quality = bvh::v2::DefaultBuilder<Nodev2>::Quality::High;
			auto bvh = bvh::v2::DefaultBuilder<Nodev2>::build(thread_pool, bboxes, centers, config);

			for (int i = 0; i < bvh.nodes.size(); i++) {
				Node node;
				auto min = bvh.nodes[i].get_bbox().min;
				auto max = bvh.nodes[i].get_bbox().max;
				node.min = { min[0], min[1], min[2] };
				node.max = { max[0], max[1], max[2] };
				node.first = bvh.nodes[i].index.first_id;
				node.primCount = bvh.nodes[i].index.prim_count;
				bvhData.push_back(node);
			}			
#endif

			Renderer3D::AllocateMesh(totalVertices, totalIndices, 1);

			Node node;
			node.min = glm::vec4(vertices[vertexOffset].Position, 1.f);
			node.max = glm::vec4(vertices[vertexOffset].Position, 1.f);
			node.first = indexOffset;
			node.primCount = totalIndices;

			for (uint32_t i = vertexOffset + 1; i < vertices.size(); i++) {
				node.min = glm::max(node.min, vertices[i].Position);
				node.max = glm::min(node.max, vertices[i].Position);
			}
			cmd.bvhID = bvhData.size();
			bvhData.push_back(node);
			importer.FreeScene();
		}

		void processNode(const aiNode* node, const aiScene& scene, uint32_t& offset, uint32_t materialID, std::vector<Renderer3D::Vertex>& vertices, std::vector<uint32_t>& indices) {
			// process each mesh located at the current node		
			// the node object only contains indices to index the actual objects in the scene. 
			// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
			for (uint32_t m = 0; m < node->mNumMeshes; m++) {
				auto& mesh = *scene.mMeshes[node->mMeshes[m]];
				for (uint32_t i = 0; i < mesh.mNumVertices; i++)
				{
					Renderer3D::Vertex vertex;
					vertex.Position = AssimpGLMHelpers::GetGLMVec(mesh.mVertices[i]) + glm::vec3(0.5f, 0.f, 0.5f);
					vertex.Normal = AssimpGLMHelpers::GetGLMVec(mesh.mNormals[i]);
					vertex.Tangent = AssimpGLMHelpers::GetGLMVec(mesh.mTangents[i]);
					vertex.Bitangent = AssimpGLMHelpers::GetGLMVec(mesh.mBitangents[i]);
					vertex.materialID = materialID;

					if (mesh.mTextureCoords[0])
						vertex.TexCoords = glm::vec2(mesh.mTextureCoords[0][i].x, mesh.mTextureCoords[0][i].y);

					vertices.push_back(vertex);
				}

				for (uint32_t i = 0; i < mesh.mNumFaces; i++)
				{
					aiFace& face = mesh.mFaces[i];
					for (uint32_t j = 0; j < face.mNumIndices; j++)
						indices.push_back(face.mIndices[j] + offset + Renderer3D::GetVertexSize());
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