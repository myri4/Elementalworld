#pragma once

#include "Block.h"

namespace wc {	
	struct Biome {
		Biome() = default;

		BlockID topBlock = 0;
		float minTemp = 0.f;
		float maxTemp = 1.f;

		float minMois = 0.f;
		float maxMois = 1.f;
	};
}