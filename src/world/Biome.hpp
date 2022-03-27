#pragma once

#include "Block.hpp"

namespace wc {	
	class Biome {
	public:
		Biome() {}

		BlockID topBlock = 0;
		float minTemp = 0.f;
		float maxTemp = 1.f;

		float minMois = 0.f;
		float maxMois = 1.f;
	};
	uint8_t numBiomes = 0;
	Biome biomeMap[40];

	void AddBiome(const Biome& biome) {
		biomeMap[numBiomes] = biome;
		numBiomes++;
	}
}