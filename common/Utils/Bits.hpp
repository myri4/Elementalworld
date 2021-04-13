#ifndef BITS_HPP
#define BITS_HPP

#include <glm/glm.hpp>

bool bitEnabled(const uint8_t& flags, const uint8_t& bit) {
	return (flags >> bit) & 1;
}

void disableBit(uint8_t& flags, const uint8_t& bit) {
	flags = flags & (~bit); // disables flag
}

void enableBit(uint8_t& flags, const uint8_t& bit) {
	flags = flags | bit; // enables flag
}

#endif