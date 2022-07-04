#pragma once
#include "Block.h"

namespace wc {

	//int to1D(const glm::vec3& pos) { return (pos.z * chunkSize * chunkSize) + (pos.y * chunkSize) + pos.x; }
	int to1D(const int& x, const int& y, const int& z) { return (z * chunkSize * chunkSize) + (y * chunkSize) + x; }
	glm::ivec3 to3D(const int& idx, const glm::ivec3& size) {
		int i = idx;
		int z = i / (size.x * size.y);
		i -= (z * size.x * size.y);
		int y = i / size.x;
		int x = i % size.x;
		return glm::ivec3(x, y, z);
	}

	struct Chunk {
		glm::ivec3 position = glm::ivec3(0);
		BlockID data[chunkSize][chunkSize][chunkSize] = { 0 };
		int16_t neighborPos[3] = { -1,-1,-1 };
		int16_t neighborNeg[3] = { -1,-1,-1 };

		bool used : 1; // Should the chunk be saved
		bool generated : 1;
		bool canBeUpdated : 1;
		bool generatedStructures : 1;

		Chunk() {
			used = false;
			generated = true;
			canBeUpdated = false;
			generatedStructures = true;
		}
	};

	glm::ivec3 getBlockPos(const int& x, const int& y, const int& z)
	{
		return { (chunkSize + (x % chunkSize)) % chunkSize,
				(chunkSize + (y % chunkSize)) % chunkSize,
				(chunkSize + (z % chunkSize)) % chunkSize };
	}

	glm::ivec3 getBlockPos(const glm::ivec3& pos)
	{
		return getBlockPos(pos.x, pos.y, pos.z);
	}

	glm::ivec3 getChunkPos(const int& x, const int& y, const int& z)
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