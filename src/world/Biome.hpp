#pragma once

#include "Block.hpp"

namespace wc {	
	enum BiomeType { DESERT, PLAINS, SNOW_PEAK };

	class Biome {
	public:
		Biome() {}

		BlockID topBlock = 0;
		float minTemp = 0.f;
		float maxTemp = 1.f;

		float minMois = 0.f;
		float maxMois = 1.f;

		bool trees = false; //for now
	};	
}