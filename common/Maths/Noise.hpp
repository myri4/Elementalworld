#ifndef NOISE_HPP
#define NOISE_HPP

#include <wclibs/pch.hpp>

struct Noise {
	uint8_t octaves = 0;
	float scale = 0;
	float multiplier = 0;
	float persistance = 0;
	float lacunarity = 0;
	int16_t seed = 0;
	float getNoiseFor(const int& voxelX, const int& voxelZ)
	{
		if (octaves > 9) octaves = 9;
		if (octaves < 1) octaves = 1;

		float amplitude = 1.0f;
		float frequency = 1.0f;
		float noiseValue = 0.0f;

		for (uint8_t i = 0; i < octaves; i++) {
			float perlinValue = (glm::perlin(glm::vec3(voxelX / scale * frequency, 
													   voxelZ / scale * frequency, seed)) + 1) / 2;
			noiseValue += perlinValue * amplitude;

			amplitude *= persistance;
			frequency *= lacunarity;
		}

		return noiseValue * multiplier;
	}
};

#endif