#pragma once
#include <pch.h>
#include "AssetManager.h"

namespace wc {

	static const uint32_t MaxLineVertexCount = 10000 * 2;

	struct LineVertex {
		glm::vec4 pos = glm::vec4(0.f);
		glm::vec4 color = glm::vec4(0.f);

		LineVertex() = default;
		LineVertex(const glm::vec3& position, const glm::vec4& Color) {
			pos = glm::vec4(position, 0.f);
			color = Color;
		}
	};

	class LineBatcher {
		uint32_t IndexCount = 0;
		uint32_t byteOffset = 0;

		wc::Buffer lineBuffer;

		LineVertex vertices[MaxLineVertexCount];
		wc::Shader shader;
	public:
		void Create() {
			lineBuffer.Create(sizeof(vertices), wc::STORAGE_BUFFER);
		}

		void CreatePipeline(const wc::RenderPass& renderPass, const VkDescriptorBufferInfo& ubo) {
			wc::ShaderCreateInfo createInfo;
			createInfo.vertexShader =   GetAssetPath() + "/shaders/Line3D.vert";
			createInfo.fragmentShader = GetAssetPath() + "/shaders/Line3D.frag";
			createInfo.windowSize = window.GetSize();
			createInfo.renderPass = renderPass;
			createInfo.blending = false;
			createInfo.depthTest = true;
			createInfo.invertY = true;
			createInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			createInfo.cachePath = GetCachedAssetPath() + "/shaders/LineShader.bin";
			shader.Create(createInfo);


			wc::DescriptorWriter writer;

			writer.dstSet = shader.descriptorSet;
			writer.write_buffer(0, ubo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
			writer.write_buffer(1, lineBuffer.GetDescriptorInfo(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

			wc::UpdateDescriptorSets((uint32_t)writer.writes.size(), writer.writes.data());
		}

		void DestroyPipeline() {
			shader.Destroy();
		}

		void Destroy() {
			lineBuffer.Destroy();
			DestroyPipeline();
		}

		void DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color = glm::vec4(1.f)) {
			if (IndexCount >= MaxLineVertexCount) Flush();

			vertices[IndexCount + 0] = LineVertex(start, color);
			vertices[IndexCount + 1] = LineVertex(end, color);

			IndexCount += 2;
		}

		void DrawOutlineCube(const glm::vec3& pos, const glm::vec3& size, const glm::vec4& color) {
			DrawLine(pos, pos + glm::vec3(0.f, size.y, 0.f), color);
			DrawLine(pos, pos + glm::vec3(size.x, 0.f, 0.f), color);
			DrawLine(pos + glm::vec3(size.x, 0.f, 0.f), pos + glm::vec3(size.x, size.y, 0.f), color);
			DrawLine(pos + glm::vec3(size.x, size.y, 0.f), pos + glm::vec3(0.f, size.y, 0.f), color);

			DrawLine(pos + glm::vec3(0.f, 0.f, size.z), pos + glm::vec3(0.f, size.y, size.z), color);
			DrawLine(pos + glm::vec3(0.f, 0.f, size.z), pos + glm::vec3(size.x, 0.f, size.z), color);
			DrawLine(pos + glm::vec3(size.x, 0.f, size.z), pos + glm::vec3(size.x, size.y, size.z), color);
			DrawLine(pos + size, pos + glm::vec3(0.f, size.y, size.z), color);

			DrawLine(pos + glm::vec3(0.f, 0.f, size.z), pos, color);
			DrawLine(pos + glm::vec3(size.x, 0.f, size.z), pos + glm::vec3(size.x, 0.f, 0.f), color);

			DrawLine(pos + glm::vec3(0.f, size.y, size.z), pos + glm::vec3(0.f, size.y, 0.f), color);
			DrawLine(pos + size, pos + glm::vec3(size.x, size.y, 0.f), color);
		}

		void DrawOutlineCube(const AABB& aabb, const glm::vec4& color) {
			DrawOutlineCube(aabb.position, aabb.size, color);
		}

		void Flush(const bool render = true) {
			if (!IndexCount) return;

			if (render) {
				lineBuffer.SetData(vertices, sizeof(LineVertex) * IndexCount);
				wc::CommandBuffer& cmd = RendererContext::mainCommandBuffer;
				shader.Bind(cmd);
				cmd.Draw(IndexCount);
			}
			byteOffset = 0;
			IndexCount = 0;
		}
	};
}