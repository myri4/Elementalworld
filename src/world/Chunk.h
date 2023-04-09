#pragma once
#include "Block.h"

namespace wc {

	constexpr uint16_t chunkSize = 16;
	constexpr uint32_t chunkVolume = chunkSize * chunkSize * chunkSize;

	using ChunkID = uint16_t;

	struct Chunk {
		glm::ivec3 position = glm::ivec3(0);
		BlockID data[chunkSize][chunkSize][chunkSize] = { 0 };
		glm::vec3 boxStart;
		glm::vec3 boxEnd;
		bool used : 1; // Should the chunk be saved
		bool generated : 1;
		bool canBeUpdated : 1;
		bool generatedStructures : 1;
		bool empty : 1;

		Chunk() {
			used = false;
			generated = true;
			canBeUpdated = false;
			generatedStructures = true;
			empty = true;
		}
	};

	glm::ivec3 getBlockPos(int x, int y, int z)
	{
		return { (chunkSize + (x % chunkSize)) % chunkSize,
				(chunkSize + (y % chunkSize)) % chunkSize,
				(chunkSize + (z % chunkSize)) % chunkSize };
	}

	glm::ivec3 getBlockPos(const glm::ivec3& pos)
	{
		return getBlockPos(pos.x, pos.y, pos.z);
	}

	glm::ivec3 getChunkPos(int x, int y, int z)
	{
		glm::ivec3 res = {
			x < 0 ? ((x - chunkSize) / chunkSize) : (x / chunkSize),
			y < 0 ? ((y - chunkSize) / chunkSize) : (y / chunkSize),
			z < 0 ? ((z - chunkSize) / chunkSize) : (z / chunkSize),
		};

		glm::ivec3 localPos = getBlockPos(x, y, z);
		if (localPos.x == 0 && x < 0) res.x++;
		if (localPos.y == 0 && y < 0) res.y++;
		if (localPos.z == 0 && z < 0) res.z++;
		return res;
	}

	glm::ivec3 getChunkPos(const glm::ivec3& pos)
	{
		return getChunkPos(pos.x, pos.y, pos.z);
	}

}