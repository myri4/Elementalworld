#ifndef NOISE_HPP
#define NOISE_HPP

#include <wclibs/pch.hpp>

struct Noise {
private:
    float getNoise(int n) const noexcept
    {
        n += seed;
        n = (n << 13) ^ n;
        float newN = (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;

        return 1.0 - ((float)newN / 1073741824.0);
    }

    float getNoise(const float& x, const float& z) const noexcept
    {
        return getNoise(x + z * 57.f);
    }

    float lerp(const float& a, const float& b, const float& z) const noexcept
    {
        float mu2 = (1 - glm::cos(z * glm::pi<float>())) / 2;
        return (a * (1 - mu2) + b * mu2);
    }

    float noise(const float x, const float z) const noexcept
    {
        float floorX = glm::floor(x); // This is kinda a cheap way to floor a float integer.
        float floorZ = glm::floor(z);

        float s = getNoise(floorX, floorZ);
        float t = getNoise(floorX + 1, floorZ);
        float u = getNoise(floorX, floorZ + 1); // Get the surrounding values to calculate the transition.
        float v = getNoise(floorX + 1, floorZ + 1);

        float rec1 = lerp(s, t, x - floorX); // Interpolate between the values.
        float rec2 = lerp(u, v, x - floorX); // Here we use x-floorX, to get 1st dimension. Don't mind the x-floorX thingie, it's part of the cosine formula.
        float rec3 = lerp(rec1, rec2, z - floorZ); // Here we use y-floorZ, to get the 2nd dimension.
        return rec3;
    }
public:
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
			float perlinValue = (noise(voxelX / scale * frequency, voxelZ / scale * frequency) + 1) / 2;
			noiseValue += perlinValue * amplitude;

			amplitude *= persistance;
			frequency *= lacunarity;
		}

		return noiseValue * multiplier;
	}
};

#endif