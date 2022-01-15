#pragma once

#include "Block.hpp"

namespace wc {	
	enum BiomeType { DESERT, PLAINS, SNOW_PEAK };

	class Biome {
	public:
		uint8_t numberFloraBlocks = 0;
		Biome() {}

		BlockID topBlock = 0;
		float minTemp = 0.f;
		float maxTemp = 1.f;

		float minMois = 0.f;
		float maxMois = 1.f;

		bool trees = false; //for now
		bool underWater = false;
		
		//RarityTable floraTables[8];
		
		void addFloraTable(const RarityTable& table) {
		//	floraTables[numberFloraBlocks] = table;
			numberFloraBlocks++;
		}
	};	
}