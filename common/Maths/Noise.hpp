#ifndef NOISE_HPP
#define NOISE_HPP

#include <wclibs/pch.hpp>

struct Noise {
	uint8_t octaves = 5;
	float smoothnes = 90.0f;
	float multiplier = 32.0f;
	int16_t seed = 10;
	float getNoiseFor(const glm::vec2& pos, const glm::vec2& chunkPosition, const int8_t& chunkSize)
	{
		int voxelX = pos.x + chunkPosition.x * chunkSize;
		int voxelZ = pos.y + chunkPosition.y * chunkSize;

		float value = glm::perlin(glm::vec3(voxelX / smoothnes + seed, voxelZ / smoothnes + seed, seed));
		
		value = (value + 1) / 2;
		value *= multiplier + multiplier;
		return value;
	}
};

#endif 