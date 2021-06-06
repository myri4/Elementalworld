#ifndef NOISE_HPP
#define NOISE_HPP

#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>

struct Noise {
private:

    float getNoise(const int& x, const int& z) const
    {
		int n = x + z * 57;
		n += seed;
		n = (n << 15) ^ n;
		int newN = (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;

		return 1.f - (newN * 9.313226e-10f); // 1073741824
    }

    float noise(const float& x, const float& z) const
    {
        int floorX = (int)glm::floor(x);
        int floorZ = (int)glm::floor(z);

        float s = getNoise(floorX, floorZ);
        float t = getNoise(floorX + 1, floorZ);
        float u = getNoise(floorX, floorZ + 1); 
        float v = getNoise(floorX + 1, floorZ + 1);

        float rec1 = glm::lerp(s, t, x - floorX);
        float rec2 = glm::lerp(u, v, x - floorX);
        float rec3 = glm::lerp(rec1, rec2, z - floorZ);
        return rec3;
    }
public:
	uint8_t octaves = 0;
	float scale = 0;
	float multiplier = 0;
	float persistance = 0;
	float lacunarity = 0;
	int16_t seed = 0;
	float getNoiseFor(const int& voxelX, const int& voxelZ) {
		float amplitude = 1.f;
		float frequency = 1.f;
		float noiseValue = 0.f;

		for (uint8_t i = 0; i < octaves; i++) {
			float scaleFreq = scale * frequency;
			float perlinValue = (noise(voxelX * scaleFreq, voxelZ * scaleFreq) + 1) * 0.5f;
			noiseValue += perlinValue * amplitude;

			amplitude *= persistance;
			frequency *= lacunarity;
		}

		return noiseValue * multiplier;
	}

	float get3DNoiseFor(const int& voxelX, const int& voxelY, const int& voxelZ)
	{
        float noiseX = noise(voxelY * scale, voxelZ * scale);
        float noiseY = noise(voxelX * scale, voxelZ * scale);
        float noiseZ = noise(voxelX * scale, voxelY * scale);

        float abc = noiseX + noiseY + noiseZ;
        return abc * multiplier * 0.1666666666666667f; //  / 6
	}
};

#endif