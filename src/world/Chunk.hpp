#pragma once

#include <gl/VertexBuffer.hpp>
#include <gl/Vertex.hpp>
#include <glm/glm.hpp>
#include <array>

//OpenGL Memory Buffer Variables
static const size_t chunkSize = 32;

static const size_t MaxFaceCount = 125;
static const size_t MaxVertexCount = MaxFaceCount * 4;
static const size_t MaxIndexCount = MaxFaceCount * 6;

namespace wc {
int to1D(const glm::vec3& pos) { return (pos.z * chunkSize * chunkSize) + (pos.y * chunkSize) + pos.x; }
glm::vec3 to3D(const int& idx) {
	int i = idx;
	int z = i / (chunkSize * chunkSize);
	i -= (z * chunkSize * chunkSize);
	int y = i / chunkSize;
	int x = i % chunkSize;
	return glm::vec3(x, y, z);
}
class Chunk {
public: // Variables
	int32_t chunkPosition = 0;
	int8_t chunkData[chunkSize * chunkSize * chunkSize];
	bool used = false;

	gl::VertexBuffer chunkMeshBuffer;
	gl::Vertex chunkMesh[MaxVertexCount];

	uint32_t IndexCount = 0;
	uint32_t offset = 0;

public: // Functions 
	Chunk() {}
	Chunk(const glm::vec3& pos) { Create(pos); }
	~Chunk() {}
	void Create(const glm::vec3& pos) {
		chunkPosition = to1D(pos);
		//Configuring the vertex array
		chunkMeshBuffer.Create(nullptr, MaxVertexCount * sizeof(gl::Vertex), GL_DYNAMIC_DRAW);
		gl::VertexAttribPointer(0, 3, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, Position));  // position attribute
		gl::VertexAttribPointer(1, 2, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, TexCoords)); // texture coord attribute
		gl::VertexAttribPointer(2, 3, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, Normal));    // Normal attribute
	}
	void Create(const int32_t& pos) {
		chunkPosition = pos;
		//Configuring the vertex array
		chunkMeshBuffer.Create(nullptr, MaxVertexCount * sizeof(gl::Vertex), GL_DYNAMIC_DRAW);
		gl::VertexAttribPointer(0, 3, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, Position));  // position attribute
		gl::VertexAttribPointer(1, 2, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, TexCoords)); // texture coord attribute
		gl::VertexAttribPointer(2, 3, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, Normal));    // Normal attribute
	}
};
}