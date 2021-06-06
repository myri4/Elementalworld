#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <gl/VertexBuffer.hpp>
#include <gl/VertexArray.hpp>
#include "Block.hpp"

//OpenGL Memory Buffer Variables
//@Todo try with size_t 
const uint8_t chunkSize = 16;
const uint16_t chunkVolume = chunkSize * chunkSize * chunkSize;
const uint8_t chunkSizeMinusOne = chunkSize - 1;
const float cs = 1.f / chunkSize;

const uint32_t MaxFaceCount = chunkVolume;
const uint32_t MaxVertexCount = MaxFaceCount * 4;

typedef uint16_t ChunkID; // This represents the chunk id in the chunk array

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
	glm::ivec3 chunkPos = glm::ivec3(0);
	BlockID chunkData[chunkSize][chunkSize][chunkSize] = { 0 };

	bool used : 1;
	bool generated : 1;
	bool canBeUpdated : 1;
	bool empty : 1;

	int16_t neighborXpos = -1;
	int16_t neighborYpos = -1;
	int16_t neighborZpos = -1;
						   
	int16_t neighborXneg = -1;
	int16_t neighborYneg = -1;
	int16_t neighborZneg = -1;

	gl::VertexBuffer chunkMeshBuffer;
	gl::VertexArray chunkMeshArray;	

public: // Functions 
	Chunk() {
		used = false;
		generated = false;
		canBeUpdated = true;
		empty = false;
	}
};

glm::ivec3 getBlockPos(const int& x, const int& y, const int& z)
{
	return { x & chunkSizeMinusOne, y & chunkSizeMinusOne, z & chunkSizeMinusOne };
}

glm::ivec3 getBlockPos(const glm::ivec3& pos)
{
	return { pos.x & chunkSizeMinusOne, pos.y & chunkSizeMinusOne, pos.z & chunkSizeMinusOne };
}

glm::ivec3 getChunkPos(const int& x, const int& y, const int& z)
{
	return glm::ivec3( x * cs, y * cs, z * cs );
}

glm::ivec3 getChunkPos(const glm::ivec3& pos)
{
	return glm::ivec3( pos.x * cs, pos.y * cs, pos.z * cs );
}
}
#endif