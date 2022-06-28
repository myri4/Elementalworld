#pragma once

#include "Block.h"
#include <Utils/List.h>

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
	List<Biome, 40> biomeMap;
}