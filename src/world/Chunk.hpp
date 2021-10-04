#pragma once
#include <gl/VertexArray.hpp>
#include <gl/Buffer.hpp>
#include "Block.hpp"

//OpenGL Memory Buffer Variables
//@Todo try with size_t 
const uint16_t chunkSize = 16;
const uint32_t chunkVolume = chunkSize * chunkSize * chunkSize;

const uint32_t MaxFaceCount = chunkSize * chunkSize * chunkSize;
const uint32_t MaxVertexCount = MaxFaceCount * 4;

typedef uint16_t ChunkID; // This represents the chunk id in the chunk array
typedef glm::ivec3 chunkPosV;

namespace wc {

//int to1D(const glm::vec3& pos) { return (pos.z * chunkSize * chunkSize) + (pos.y * chunkSize) + pos.x; }
int to1D(const int& x, const int& y, const int& z) { return (z * chunkSize * chunkSize) + (y * chunkSize) + x; }
glm::ivec3 to3D(const int& idx, const glm::ivec3& size = glm::ivec3(chunkSize)) {
	int i = idx;
	int z = i / (size.x * size.y);
	i -= (z * size.x * size.y);
	int y = i / size.x;
	int x = i % size.x;
	return glm::ivec3(x, y, z);
}

class Chunk {
public: // Variables
	uint32_t IndexCount = 0;
	chunkPosV chunkPos = chunkPosV(0);
	BlockID chunkData[chunkSize][chunkSize][chunkSize] = { 0 };

	bool used : 1;
	bool generated : 1;
	bool canBeUpdated : 1;
	bool empty : 1;
	bool generatedStructures : 1;

	int16_t neighborXpos = -1;
	int16_t neighborYpos = -1;
	int16_t neighborZpos = -1;
						   
	int16_t neighborXneg = -1;
	int16_t neighborYneg = -1;
	int16_t neighborZneg = -1;

	gl::VertexBuffer chunkMeshBuffer;
public: // Functions 
	Chunk() {
		used = false;
		generated = false;
		canBeUpdated = false;
		empty = true;
		generatedStructures = false;
	}
};

glm::ivec3 getBlockPos(const int& x, const int& y, const int& z)
{
	return {(chunkSize + (x % chunkSize)) % chunkSize,
			(chunkSize + (y % chunkSize)) % chunkSize,
			(chunkSize + (z % chunkSize)) % chunkSize };
}

glm::ivec3 getBlockPos(const glm::ivec3& pos)
{
	return getBlockPos(pos.x, pos.y, pos.z);
}

glm::ivec3 getChunkPos(const int& x, const int& y, const int& z)
{
	return {
		x < 0 ? ((x - chunkSize) / chunkSize) : (x / chunkSize),
		y < 0 ? ((y - chunkSize) / chunkSize) : (y / chunkSize),
		z < 0 ? ((z - chunkSize) / chunkSize) : (z / chunkSize),
	};
}

glm::ivec3 getChunkPos(const glm::ivec3& pos)
{
	return getChunkPos(pos.x, pos.y, pos.z);
}
}