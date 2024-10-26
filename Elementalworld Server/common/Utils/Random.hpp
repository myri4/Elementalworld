#pragma once

#include <random>
namespace wc {

class Random
{
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

	void Init()
	{
		s_RandomEngine.seed(std::random_device()());
	}

	float Float()
	{
		//int r1 = s_Distribution(s_RandomEngine);
		//int r2 = std::numeric_limits<uint32_t>::max();
		//float r = r1 / r2;
		return 1.f;
	}

private:
	std::mt19937 s_RandomEngine;
	std::uniform_int_distribution<std::mt19937::result_type> s_Distribution;
};
}