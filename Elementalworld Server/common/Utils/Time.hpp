#pragma once

#include <chrono>
#include "Log.hpp"

namespace wc {
	class Clock {
	private:
		std::chrono::time_point<std::chrono::steady_clock> start, end;
	public:

		Clock() {
			restart();
		}

		float restart() {
			end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float> dur = end - start;

			start = std::chrono::high_resolution_clock::now();
			return dur.count();
		}
	};
	class Timer {
	private:
		std::chrono::time_point<std::chrono::steady_clock> start, end;
		const char* op;
	public:

		Timer(const char* opn) {
			op = opn;
			start = std::chrono::high_resolution_clock::now();
			end = std::chrono::high_resolution_clock::now();
		}

		~Timer() {
			end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float> dur = end - start;
			float duration = dur.count() * 1000.0f;

			WC_INFO("{0} took {1}ms!", op, duration);
		}
	};
}