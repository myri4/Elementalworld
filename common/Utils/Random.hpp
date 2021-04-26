#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <random>

namespace wc{
class Random{
public:
	uint32_t seed = 0;
	uint32_t asInt()
	{
		seed += 0xe120fc15;
		uint32_t tmp;
		tmp = seed * 0x4a39b70du;
		uint32_t m1 = (tmp >> 31u) ^ tmp; // 32
		tmp = m1 * 0x12fad5c9u;
		uint32_t m2 = (tmp >> 31u) ^ tmp; // 32
		return m2;
	}

};
}
#endif