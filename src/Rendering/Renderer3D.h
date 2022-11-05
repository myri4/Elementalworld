#pragma once

#include <wc/vk/Buffer.h>
#include <wc/vk/Images.h>
#include <wc/Shader.h>
#include "../Globals.h"
#include <stdint.h>

struct Mesh {
	uint32_t vertexOffset = 0;
	uint32_t indexOffset = 0;
};

namespace Renderer3D {
	namespace {
		uint32_t vertexSize = 0;
		uint32_t indexSize = 0;
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
	}

	void Bind(const wc::CommandBuffer& cmd) {
		shader.Bind(cmd);
		cmd.BindIndexBuffer(indexBuffer);
		cmd.BindVertexBuffer(vertexBuffer);
	}

	void Destroy() {
		shader.Destroy();
		vertexBuffer.Destroy();
		indexBuffer.Destroy();
		depthBuffer.Destroy();
	}
}