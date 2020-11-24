#pragma once

#include <chrono>
#include <wclibs/Core.hpp>
#include "Log.hpp"

namespace wc {
class Clock {
private:
	std::chrono::time_point<std::chrono::steady_clock> start, end;
public:
	
	Clock() {}

	float restart() {
		end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> dur = end - start;

		start = std::chrono::high_resolution_clock::now();
		return  dur.count();

	}

};
}