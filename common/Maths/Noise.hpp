#ifndef NOISE_HPP
#define NOISE_HPP

#include <wclibs/pch.hpp>

struct Noise {
	uint8_t octaves = 4;
	float scale = 90.f;
	float multiplier = 32.f;
	float persistance = 0.4f;
	float lacunarity = 2.f;
	int16_t seed = 10;
	float getNoiseFor(const glm::vec2& pos, const glm::vec2& chunkPosition, const int8_t& chunkSize)
	{
		float amplitude = 1.0f;
		float frequency = 1.0f;
		float noiseValue = 0.0f;

		int voxelX = pos.x + chunkPosition.x * chunkSize;
		int voxelZ = pos.y + chunkPosition.y * chunkSize;

		for (int8_t i = 0; i < octaves; i++) {
			float sampleX = voxelX / scale * frequency;
			float sampleZ = voxelZ / scale * frequency;
			float perlinValue = (glm::perlin(glm::vec3(sampleX, sampleZ, seed)) + 1) / 2;
			noiseValue += perlinValue * amplitude;

			amplitude *= persistance;
			frequency *= lacunarity;
		}

		noiseValue *= multiplier + multiplier;
		return noiseValue;
	}
};

#endif