#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <gl/Vertex.hpp>
#include <array>

//OpenGL Memory Buffer Variables
static const size_t chunkSize = 16;

static const size_t MaxFaceCount = chunkSize * chunkSize * 7;
static const size_t MaxVertexCount = MaxFaceCount * 4;

namespace wc {
int to1D(const glm::vec3& pos) { return (pos.z * chunkSize * chunkSize) + (pos.y * chunkSize) + pos.x; }
int to1D(const int& x, const int& y, const int& z) { return (z * chunkSize * chunkSize) + (y * chunkSize) + x; }
glm::vec3 to3D(const int& idx, const glm::ivec3& size = glm::vec3(chunkSize)) {
	int i = idx;
	int z = i / (size.x * size.y);
	i -= (z * size.x * size.y);
	int y = i / size.x;
	int x = i % size.x;
	return glm::vec3(x, y, z);
}


class Chunk {
public: // Variables
	uint32_t IndexCount = 0;
	uint32_t fIndexCount = 0;
	int32_t chunkPosition = 0;
	int8_t chunkData[chunkSize][chunkSize][chunkSize];
	bool used = false;
	bool generated = false;

	gl::VertexBuffer chunkMeshBuffer;
	gl::VertexArray chunkMeshArray;

	gl::VertexBuffer chunkFluidMeshBuffer;
	gl::VertexArray chunkFluidMeshArray;

public: // Functions 
	Chunk() {}
	Chunk(const int32_t& pos) { Create(pos); }
	~Chunk() {}
	void Create(const int32_t& pos) {
		chunkPosition = pos;
		//Configuring the vertex array
		chunkFluidMeshArray.Create();
		chunkFluidMeshArray.Bind();
		chunkFluidMeshBuffer.Create(nullptr, MaxVertexCount * sizeof(gl::Vertex), GL_DYNAMIC_DRAW);
		gl::VertexAttribPointer(0, 3, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, Position));  // position attribute
		gl::VertexAttribPointer(1, 2, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, TexCoords)); // texture coord attribute

		chunkMeshArray.Create();
		chunkMeshArray.Bind();
		chunkMeshBuffer.Create(nullptr, MaxVertexCount * sizeof(gl::Vertex), GL_DYNAMIC_DRAW);
		gl::VertexAttribPointer(0, 3, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, Position));  // position attribute
		gl::VertexAttribPointer(1, 2, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, TexCoords)); // texture coord attribute
		
	}

	glm::vec3 getPosition() { return to3D(chunkPosition);}
};
}

#endif