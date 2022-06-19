#pragma once

#include <glm/glm.hpp>
#include "../../src/world/Block.hpp"

namespace wc {
	namespace mem {
		uint8_t memory[1024 * 3];
		uint32_t currentPosition = 0;

		void* getCurrPtr() {
			return &memory[currentPosition];
		}

		inline void incPtr(const uint32_t& size) {
			currentPosition += size;
		}
	}
}