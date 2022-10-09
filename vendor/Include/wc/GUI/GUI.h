#pragma once

#include <glm/glm.hpp>

#include "../vk/Buffer.h"
#include "../vk/Images.h"
#include "../Shader.h"
#include "../vk/Renderpass.h"
#include "../vk/RendererContext.h"

#include <glm/gtc/type_ptr.hpp>

namespace wc {

	class RenderInterface
	{
		static const uint32_t MaxQuadCount = 100;
		static const uint32_t MaxQuadVertexCount = MaxQuadCount * 4;
		static const uint32_t MaxQuadIndexCount = MaxQuadCount * 6;

		static const uint8_t MaxTextures = 32;

		bool transformEnabled = false;

		wc::Buffer VBO;
		wc::Buffer EBO;

		wc::Shader shader;


		std::array<wc::Texture, 100> m_Textures;
		uint32_t m_NumTextures = 0;


		struct Vertex2D {
			glm::vec2 position = glm::vec2(0.f);
			uint32_t color = 0xFFFFFFFF;
			glm::vec3 tex_coord = glm::vec3(0.f);

			Vertex2D() = default;
			Vertex2D(const glm::vec2& pos, const glm::vec3& texCoord) : position(pos), tex_coord(texCoord) { }

			static wc::VertexInputDescription get_vertex_description() {
				wc::VertexInputDescription description;

				VkVertexInputBindingDescription mainBinding = {};
				mainBinding.binding = 0;
				mainBinding.stride = sizeof(Vertex2D);
				mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

				description.bindings.push_back(mainBinding);

				VkVertexInputAttributeDescription positionAttribute = {};
				positionAttribute.binding = 0;
				positionAttribute.location = 0;
				positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
				positionAttribute.offset = offsetof(Vertex2D, position);

				VkVertexInputAttributeDescription colorAttribute = {};
				colorAttribute.binding = 0;
				colorAttribute.location = 1;
				colorAttribute.format = VK_FORMAT_R32_UINT;
				colorAttribute.offset = offsetof(Vertex2D, color);

				VkVertexInputAttributeDescription texCoordAttribute = {};
				texCoordAttribute.binding = 0;
				texCoordAttribute.location = 2;
				texCoordAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
				texCoordAttribute.offset = offsetof(Vertex2D, tex_coord);

				description.attributes.push_back(positionAttribute);
				description.attributes.push_back(colorAttribute);
				description.attributes.push_back(texCoordAttribute);
				return description;
			}
		};

		uint32_t IndexCount = 0;
		uint32_t TextureSlots[MaxTextures] = { 0 };
		uint32_t VertexCount = 0;
		uint8_t TextureSlotIndex = 0;

		uint32_t getTexture(const uint32_t& texID) {
			uint32_t textureIndex = 0;

			for (uint32_t i = 0; i < TextureSlotIndex; i++) {
				if (TextureSlots[i] == texID) {
					textureIndex = i;
					break;
				}
			}

			if (textureIndex == 0) {
				textureIndex = TextureSlotIndex;
				TextureSlots[TextureSlotIndex] = texID;
				TextureSlotIndex++;
			}
			return textureIndex;
		}

		wc::CPUBuffer<uint32_t> indices;
		wc::CPUBuffer<Vertex2D> vertices;
	public:
		glm::vec2 windowSize = glm::vec2(0.f);
		uint32_t whiteTexture;

		void Create(const wc::RenderPass& renderPass) {

			wc::ShaderCreateInfo createInfo;
			createInfo.vertexShader = "resourcepacks/default/shaders/RmlRenderer.vert";
			createInfo.fragmentShader = "resourcepacks/default/shaders/RmlRenderer.frag";
			createInfo.windowSize = windowSize;
			createInfo.renderPass = renderPass;
			createInfo.vertexDescription = Vertex2D::get_vertex_description();
			createInfo.blending = true;
			createInfo.depthTest = false;
			createInfo.invertY = true;
			shader.Create(createInfo);

			EBO.Create(sizeof(uint32_t) * MaxQuadIndexCount, wc::INDEX_BUFFER);
			VBO.Create(sizeof(Vertex2D) * MaxQuadVertexCount, wc::VERTEX_BUFFER);

			indices.Create(sizeof(uint32_t) * MaxQuadIndexCount); indices.Map();
			vertices.Create(sizeof(Vertex2D) * MaxQuadVertexCount); vertices.Map();

			uint32_t color = 0xFFFFFFFF;
			GenerateTexture(whiteTexture, &color, {1, 1});

			std::vector<VkDescriptorImageInfo> imageInfos;
			imageInfos.reserve(MaxTextures);
			for (uint32_t i = 0; i < MaxTextures; i++)
				imageInfos.emplace_back(m_Textures[whiteTexture].GetDescriptorData());

			VkWriteDescriptorSet newWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };

			newWrite.descriptorCount = imageInfos.size();
			newWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			newWrite.pImageInfo = imageInfos.data();
			newWrite.dstBinding = 0;
			newWrite.dstSet = shader.descriptorSet;

			wc::UpdateDescriptorSets(1, &newWrite);
		}

		void Destroy() {
			VBO.Destroy();
			EBO.Destroy();
			vertices.Unmap();
			indices.Unmap();
			vertices.Destroy();
			indices.Destroy();

			for (uint32_t i = 0; i < m_NumTextures; i++) m_Textures[i].Destroy();
			shader.Destroy();
		}

		void RenderGeometry(Vertex2D* p_vertices, int num_vertices, int* p_indices, int num_indices, uint32_t texture, const glm::vec2& translation) {
			//if (IndexCount + num_indices >= MaxQuadIndexCount || TextureSlotIndex >= MaxTextures) Flush();
			//
			//uint32_t texID = texture;
			////if (texID == 0) texID = whiteTexture;
			//float textureIndex = (float)getTexture(texID);
			//
			//for (int32_t i = 0; i < num_indices; i++) {
			//	indices[IndexCount] = p_indices[i] + VertexCount;
			//	IndexCount++;
			//}
			//
			//glm::mat4 trans = glm::mat4(1.f);
			//
			//if (transformEnabled) {
			//	transformEnabled = false;
			//	trans = transform;
			//}
			//
			//for (int32_t i = 0; i < num_vertices; i++) {
			//	RmlVertex& vertex = vertices[VertexCount];
			//	vertex.position = (glm::vec2(p_vertices[i].position.x + translation.x, p_vertices[i].position.y + translation.y) / windowSize) * 2.f - 1.f;
			//
			//	glm::vec4 Pos = glm::vec4(vertex.position.x, vertex.position.y, 0.f, 0.f) * trans;
			//	vertex.position = glm::vec2(Pos.x, Pos.y);
			//
			//	glm::vec4 color = glm::vec4(p_vertices[i].colour.red, p_vertices[i].colour.green, p_vertices[i].colour.blue, p_vertices[i].colour.alpha);
			//	vertex.color = (uint32_t)(color.r) << 24 | (uint32_t)(color.g) << 16 | (uint32_t)(color.b) << 8 | (uint32_t)(color.a);
			//
			//	vertex.tex_coord = glm::vec3(p_vertices[i].tex_coord.x, p_vertices[i].tex_coord.y, textureIndex);
			//	VertexCount++;
			//}
		}

		void DrawQuad(const glm::vec2& pos, const glm::vec2& size, const uint32_t& texID) {
			if (IndexCount >= MaxQuadIndexCount || TextureSlotIndex >= MaxTextures) Flush();

			float textureIndex = (float)getTexture(texID);

			vertices[VertexCount + 0] = Vertex2D({ pos.x + size.x, pos.y + size.y }, { 1.f, 0.f, textureIndex });
			vertices[VertexCount + 1] = Vertex2D({ pos.x,		  pos.y + size.y }, { 0.f, 0.f, textureIndex });
			vertices[VertexCount + 2] = Vertex2D({ pos.x,		  pos.y, }, { 0.f, 1.f, textureIndex });
			vertices[VertexCount + 3] = Vertex2D({ pos.x + size.x, pos.y, }, { 1.f, 1.f, textureIndex });

			for (uint8_t i = 0; i < 4; i++) {
				glm::vec2& Pos = vertices[i + VertexCount].position;
				Pos = ((vertices[i + VertexCount].position / windowSize) * 2.f - 1.f);
			}

			indices[IndexCount + 0] = VertexCount;
			indices[IndexCount + 1] = 1 + VertexCount;
			indices[IndexCount + 2] = 2 + VertexCount;

			indices[IndexCount + 3] = 2 + VertexCount;
			indices[IndexCount + 4] = 3 + VertexCount;
			indices[IndexCount + 5] = VertexCount;

			VertexCount += 4;
			IndexCount += 6;
		}

		void Flush() {
			wc::CommandBuffer& cmd = RendererContext::mainCommandBuffer;
			if (!IndexCount) return;

			indices.Unmap();
			EBO.SetData({ 0,0,sizeof(uint32_t) * MaxQuadIndexCount }, indices.GetBuffer());
			indices.Map();

			vertices.Unmap();
			VBO.SetData({ 0,0,sizeof(Vertex2D) * MaxQuadVertexCount }, vertices.GetBuffer());
			vertices.Map();

			std::vector<VkDescriptorImageInfo> imageInfos;
			imageInfos.reserve(TextureSlotIndex);
			for (uint32_t i = 0; i < TextureSlotIndex; i++)
				imageInfos.emplace_back(m_Textures[TextureSlots[i]].GetDescriptorData());

			VkWriteDescriptorSet newWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };

			newWrite.descriptorCount = imageInfos.size();
			newWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			newWrite.pImageInfo = imageInfos.data();
			newWrite.dstBinding = 0;
			newWrite.dstSet = shader.descriptorSet;

			wc::UpdateDescriptorSets(1, &newWrite);

			VkViewport viewport;
			viewport.x = 0.f;
			viewport.y = windowSize.y; // change this to 0 to invert
			viewport.width = windowSize.x;
			viewport.height = -windowSize.y; // remove the - to invert
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			
			cmd.SetViewport(viewport);

			shader.Bind(cmd);
			cmd.BindVertexBuffer(VBO);
			cmd.BindIndexBuffer(EBO);
			cmd.DrawIndexed(IndexCount);

			TextureSlotIndex = 0;
			IndexCount = 0;
			VertexCount = 0;
		}

		bool LoadTexture(uint32_t& texture_handle, glm::ivec2& texture_dimensions, const std::string& source) {
			std::string src = (std::string)source.c_str();

			wc::Texture& texture = m_Textures[m_NumTextures];

			int fnrComponents;
			auto data = stbi_load(source.c_str(), &texture_dimensions.x, &texture_dimensions.y, &fnrComponents, 0);

			if (data) {
				VkFilter filter = VK_FILTER_LINEAR;
				if (texture_dimensions.x <= 120 || texture_dimensions.y <= 120) filter = VK_FILTER_NEAREST;


				VkSamplerCreateInfo sampler = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };

				sampler.magFilter = filter;
				sampler.minFilter = filter;
				sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;


				texture.Create(glm::ivec2(texture_dimensions.x, texture_dimensions.y));
				texture.SetData(glm::ivec2(texture_dimensions.x, texture_dimensions.y), data);
				texture.SetSamplerInfo(sampler);
			}
			else WC_ERROR("Could not open file location at path {0}!", source.c_str());

			delete data; // stbi free

			texture_handle = m_NumTextures;
			m_NumTextures++;

			return true;
		}

		bool GenerateTexture(uint32_t& texture_handle, const void* source, const glm::ivec2& source_dimensions) {
			wc::Texture& texture = m_Textures[m_NumTextures];

			VkSamplerCreateInfo sampler = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };

			sampler.magFilter = VK_FILTER_NEAREST;
			sampler.minFilter = VK_FILTER_NEAREST;
			sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

			texture.Create(source_dimensions);
			texture.SetData(source_dimensions, source);
			texture.SetSamplerInfo(sampler);

			texture_handle = m_NumTextures;
			m_NumTextures++;
			return true;
		}

		uint32_t AddTextureFramebuffer(const wc::Texture& texture) {
			m_Textures[m_Textures.size() - 1] = texture;
			return m_Textures.size() - 1;
		}

		void RemoveTexture(const uint32_t& handle) {
			std::swap(m_Textures[m_NumTextures], m_Textures[handle]);
			m_NumTextures--;
		}

		void UpdateTexture(const wc::RenderableTexture& texture) {
			m_Textures[texture.handle] = texture;
		}
	} render_interface;
}