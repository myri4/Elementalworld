#pragma once

#include <wc/vk/Buffer.h>
#include <wc/vk/Images.h>
#include <wc/Shader.h>
#include "../Globals.h"
#include "AssetManager.h"
#include <stdint.h>

static const uint32_t MaxLineVertexCount = 100 * 2;

struct LineVertex {
	glm::vec4 pos = glm::vec4(0.f);
	glm::vec4 color = glm::vec4(0.f);

	LineVertex() = default;
	LineVertex(const glm::vec3& position, const glm::vec4& Color) {
		pos = glm::vec4(position, 0.f);
		color = Color;
	}
};

struct Mesh {
	uint32_t vertexOffset = 0;
	uint32_t indexOffset = 0;
};

namespace Renderer3D {
	namespace {
		uint32_t vertexSize = 0;
		uint32_t indexSize = 0;


		uint32_t lineIndexCount = 0;
		uint32_t linebyteOffset = 0;

		wc::Buffer lineBuffer;

		LineVertex lineVertices[MaxLineVertexCount];
		wc::Shader lineShader;
	}

	enum class BloomMode
	{
		Prefilter,
		Downsample,
		UpsampleFirst,
		Upsample
	};

	wc::Shader shader;
	wc::Buffer vertexBuffer;
	wc::Buffer indexBuffer;
	wc::DepthBuffer depthBuffer;


	Mesh CreateMesh(const uint32_t& vertexCount, const uint32_t& indexCount) {
		Mesh mesh;
		mesh.vertexOffset = vertexSize;
		mesh.indexOffset = indexSize;
		vertexSize += vertexCount;
		indexSize += indexCount;


		return mesh;
	}

	void BuildBuffers(const VkExtent2D& depthExtent) {
		vertexBuffer.Create(sizeof(Vertex) * vertexSize, wc::BufferUsage::VERTEX_BUFFER | wc::BufferUsage::STORAGE_BUFFER);
		indexBuffer.Create(sizeof(uint32_t) * indexSize, wc::BufferUsage::INDEX_BUFFER | wc::BufferUsage::STORAGE_BUFFER);

		depthBuffer.Create(depthExtent);

		lineBuffer.Create(sizeof(lineVertices), wc::STORAGE_BUFFER);
	}

	void CreateLinePipeline(const wc::RenderPass& renderPass, const VkDescriptorBufferInfo& ubo) {
		wc::ShaderCreateInfo createInfo;
		createInfo.vertexShader = wc::GetAssetPath() + "/shaders/Line3D.vert";
		createInfo.fragmentShader = wc::GetAssetPath() + "/shaders/Line3D.frag";
		createInfo.cachePath = wc::GetCachedAssetPath() + "/shaders/LineShader.bin";
		createInfo.windowSize = window.GetSize();
		createInfo.renderPass = renderPass;
		createInfo.blending = false;
		createInfo.depthTest = true;
		createInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		lineShader.Create(createInfo);


		wc::DescriptorWriter writer;

		writer.dstSet = lineShader.descriptorSet;
		writer.write_buffer(0, ubo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		writer.write_buffer(1, lineBuffer.GetDescriptorInfo(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

		writer.Update();
	}

	void DestroyLinePipeline() {
		lineShader.Destroy();
	}

	void Flush(const bool render = true) {
		if (!lineIndexCount) return;

		if (render) {
			lineBuffer.SetData(lineVertices, sizeof(LineVertex) * lineIndexCount);
			wc::CommandBuffer& cmd = RendererContext::mainCommandBuffer;
			lineShader.Bind(cmd);
			cmd.Draw(lineIndexCount);
		}
		linebyteOffset = 0;
		lineIndexCount = 0;
	}

	void DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color = glm::vec4(1.f)) {
		if (lineIndexCount >= MaxLineVertexCount) Flush();

		lineVertices[lineIndexCount + 0] = LineVertex(start, color);
		lineVertices[lineIndexCount + 1] = LineVertex(end, color);

		lineIndexCount += 2;
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

	void DrawOutlineCube(const wc::AABB& aabb, const glm::vec4& color) {
		DrawOutlineCube(aabb.position, aabb.size, color);
	}	

	void Bind(const wc::CommandBuffer& cmd) {
		shader.Bind(cmd);
		cmd.BindIndexBuffer(indexBuffer);
		cmd.BindVertexBuffer(vertexBuffer);
	}

	void Destroy() {
		shader.SaveCache();
		shader.Destroy();
		vertexBuffer.Destroy();
		indexBuffer.Destroy();
		depthBuffer.Destroy();


		lineBuffer.Destroy();
		lineShader.SaveCache();
		DestroyLinePipeline();
	}
}