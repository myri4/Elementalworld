#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <gl/Vertex.hpp>
#include <Utils/CustomDefs.hpp>

//OpenGL Memory Buffer Variables
//@Todo try with size_t 
static const uint32_t chunkSize = 16;

static const uint32_t MaxFaceCount = chunkSize * chunkSize * 6;
static const uint32_t MaxVertexCount = MaxFaceCount * 4;

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
	glm::vec3 chunkPos = glm::vec3(0);
	BlockID chunkData[chunkSize][chunkSize][chunkSize] = { 0 };
	bool used = false;
	bool generated = false;
	bool canBeUpdated = true;

	int16_t neighborXpos = -1;
	int16_t neighborYpos = -1;
	int16_t neighborZpos = -1;
						   
	int32_t neighborXneg = -1;
	int32_t neighborYneg = -1;
	int32_t neighborZneg = -1;

	gl::VertexBuffer chunkMeshBuffer;
	gl::VertexArray chunkMeshArray;

public: // Functions 
	Chunk() {}
	~Chunk() {}
};
}

#endif