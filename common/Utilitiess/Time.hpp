#pragma once

#include <chrono>
#include <wclibs/Core.hpp>

class Clock {
public:
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	
	Clock() {
		start = std::chrono::high_resolution_clock::now();
		end = start;
	}

	void restart() {
		end = std::chrono::high_resolution_clock::now();
	}

	float GetElapsedTime() {
		std::chrono::duration<float> dur = end - start;

		float asMs = dur.count() * 1000;
		return asMs;
	}

};

