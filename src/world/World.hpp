#pragma once

#include "Chunk.h"

namespace wc {

	struct NoiseOptions {
		int octaves;
		float amplitude;
		float smoothness;
		float roughness;
		float offset;
	};

	float getNoiseFor(const glm::vec2& pos, const glm::vec2& chunkPosition, const NoiseOptions& options, int seed) {
		int voxelX = pos.x + chunkPosition.x * chunkSize;
		int voxelZ = pos.y + chunkPosition.y * chunkSize;

		float value = 0.0f;
		float accumulatedAmps = 0;
		value = glm::simplex(glm::vec2(voxelX / 32.0f + seed, voxelZ / 32.0f + seed));

		value = (value + 1) / 2;

		return value *= 5;
	}

	glm::vec3 GetCoords() {
		return { 0,0,0 };
	}

	class World : public NonCopyable {
	public:
		//std::unordered_map<glm::vec3, Chunk> world;
		std::array<Chunk, 32> world;
		
		World(){
		
		}
		~World() {

		}

		void Update() {

		}
		//void Create() {
		//	for (int i = 0; i < world.size(); i++) {
		//		world[i].Create(glm::vec3(i, 0, 0));
		//
		//		for (uint8_t x = 0; x < chunkSize; x++)
		//			for (uint8_t z = 0; z < chunkSize; z++) {
		//				float c = getNoiseFor(glm::vec2(x,z), world[i].chunkPosition, worldNoiseOptions, 0);
		//				for (int y = 0; y < c; y++)
		//					if (y < c) world[i].setBlock(glm::vec3(x,y,z), 1);
		//			}
		//	}
		//}
	private:
		gl::Shader worldShader;
		NoiseOptions worldNoiseOptions;
	};

}