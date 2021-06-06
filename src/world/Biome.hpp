#ifndef BIOME_HPP
#define BIOME_HPP

#include "Block.hpp"

namespace wc {
	
	class Biome {
	public:
		Biome(){}
		uint32_t biomeTemperature = 0;
		uint32_t biomeThreshhold = 0;
	};

}

#endif