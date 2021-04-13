#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <gl/VertexBuffer.hpp>
#include <gl/VertexArray.hpp>
#include "Block.hpp"

//OpenGL Memory Buffer Variables
//@Todo try with size_t 
static const uint8_t chunkSize = 16;

static const uint32_t MaxFaceCount = chunkSize * chunkSize * chunkSize;
static const uint32_t MaxVertexCount = MaxFaceCount * 4;

typedef uint16_t ChunkID; // This represents the chunk id in the chunk array

namespace wc {

const uint8_t usedFlag =		 0b00000001; // 0
const uint8_t generatedFlag =	 0b00000010; // 1
const uint8_t canBeUpdatedFlag = 0b00000100; // 2
const uint8_t emptyFlag =		 0b00001000; // 3

//int to1D(const glm::vec3& pos) { return (pos.z * chunkSize * chunkSize) + (pos.y * chunkSize) + pos.x; }
//int to1D(const int& x, const int& y, const int& z) { return (z * chunkSize * chunkSize) + (y * chunkSize) + x; }
glm::vec3 to3D(const int& idx, const glm::ivec3& size = glm::ivec3(chunkSize)) {
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

	uint8_t flags = canBeUpdatedFlag | emptyFlag;

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
};

glm::vec3 getBlockPos(const int& x, const int& y, const int& z)
{
	return glm::floor(glm::vec3{ x % chunkSize, y % chunkSize, z % chunkSize });
}

glm::vec3 getBlockPos(const glm::ivec3& pos)
{
	return glm::floor(glm::vec3{ pos.x % chunkSize, pos.y % chunkSize, pos.z % chunkSize });
}

glm::vec3 getChunkPos(const int& x, const int& y, const int& z)
{
	//float cs = 1 / chunkSize;
	//return glm::floor(glm::vec3{ x * cs, y * cs, z * cs });
	return glm::floor(glm::vec3{ x / chunkSize, y / chunkSize, z / chunkSize });
}

glm::vec3 getChunkPos(const glm::vec3& pos)
{
	//float cs = 1 / chunkSize;
	//return glm::floor(glm::vec3{ pos.x * cs, pos.y * cs, pos.z * cs });
	return glm::floor(glm::vec3{ pos.x / chunkSize, pos.y / chunkSize, pos.z / chunkSize });
}

}
#endif