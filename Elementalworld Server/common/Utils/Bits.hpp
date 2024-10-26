#pragma once

template<typename T>
class Bitfield {
public:
	bool operator[](const T& bit) {
		return (flags >> bit) & 1;
	}

	void operator()(const T& bit, const bool& isTrue) {
		if (isTrue)
			flags = flags | (1 << bit); // enables flag
		else
			flags = flags & (~(1 << bit)); // disables flag
	}
private:
	T flags;
};