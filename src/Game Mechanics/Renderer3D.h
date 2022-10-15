#pragma once

#include <wc/vk/Buffer.h>
#include <wc/vk/Images.h>
#include <wc/Shader.h>
#include "../Globals.h"

struct Mesh {
	uint32_t vertexOffset = 0;
	uint32_t indexOffset = 0;
};

namespace Renderer3D {
	namespace {
		uint32_t vertexSize = 0;
		uint32_t indexSize = 0;
	}
		wc::Shader shader;
		wc::Buffer vertexBuffer;
		wc::Buffer indexBuffer;

	Mesh CreateMesh(const uint32_t& vertexCount, const uint32_t& indexCount) {
		Mesh mesh;
		mesh.vertexOffset = vertexSize;
		mesh.indexOffset = indexSize;
		vertexSize += vertexCount;
		indexSize += indexCount;

		return mesh;
	}

	void BuildBuffers() {
		vertexBuffer.Create(sizeof(Vertex) * vertexSize, wc::BufferUsage::VERTEX_BUFFER);
		indexBuffer.Create(sizeof(uint32_t) * indexSize, wc::BufferUsage::INDEX_BUFFER);
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
	}
}