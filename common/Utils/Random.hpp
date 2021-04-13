#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <random>

namespace wc{
class Random{
public:
	uint32_t seed;
	uint32_t asInt()
	{
		seed += 0xe120fc15;
		uint64_t tmp;
		tmp = (uint64_t)seed * 0x4a39b70d;
		uint32_t m1 = (tmp >> 32) ^ tmp;
		tmp = (uint64_t)m1 * 0x12fad5c9;
		uint32_t m2 = (tmp >> 32) ^ tmp;
		return m2;
	}

};
}
#endif