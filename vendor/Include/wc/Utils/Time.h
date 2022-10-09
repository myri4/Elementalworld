#pragma once

#include <chrono>
#include "Log.h"

namespace wc {
	class Clock {
		std::chrono::time_point<std::chrono::steady_clock> start, end;
	public:

		Clock() { restart(); }

		float restart() {
			end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float> dur = end - start;

			start = std::chrono::high_resolution_clock::now();
			return dur.count();
		}
	};

	class ScopeTimer {
		std::chrono::time_point<std::chrono::steady_clock> start, end;
		const char* op;
	public:

		ScopeTimer(const char* opn) {
			op = opn;
			start = std::chrono::high_resolution_clock::now();
			end = std::chrono::high_resolution_clock::now();
		}

		~ScopeTimer() {
			end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float> dur = end - start;
			float duration = dur.count() * 1000.0f;

			WC_INFO("{0} took {1}ms!", op, duration);
		}
	};

	class Timer {
		std::chrono::time_point<std::chrono::steady_clock> start;
	public:

		Timer() = default;

		void Start() {
			start = std::chrono::high_resolution_clock::now();
		}

		float getElapsedTime() {
			std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float> dur = now - start;
			return dur.count();
		}

		~Timer() {}
	};
}