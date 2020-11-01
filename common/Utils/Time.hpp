#pragma once

#include <chrono>
#include <wclibs/Core.hpp>
#include "Log.hpp"

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

class Timer {
public:
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<float> duration;
	Timer() {
		start = std::chrono::high_resolution_clock::now();
	}

	~Timer() {
		end = std::chrono::high_resolution_clock::now();
		duration = end - start;

		float dur = duration.count() * 1000.0f;
		WC_INFO("This operation took {0}ms", dur);
	}
};