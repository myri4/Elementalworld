#pragma once

#include <glm/glm.hpp>

namespace wc {
	template<typename T, size_t Size>
	class List {
		T Data[Size];
	public:
		size_t counter = 0;

		T& operator[](const size_t& index) { return Data[index]; }
		const T& operator[](const size_t& index) const { return Data[index]; }

		T* data() { return &Data[0]; }
		const T* data() const { return &Data[0]; }

		size_t push_back(const T& type) { 
			size_t index = counter;
			Data[counter] = type;
			counter++; 
			return index;
		}

		size_t size() const { return counter; }
		constexpr size_t allocated_size() const { return Size; }
		constexpr size_t byte_size() const { return Size * sizeof(T); }
	};

	template<typename T, size_t Size>
	class PointerList {
	public:
		size_t counter = 0;
		T* Data = nullptr;

		T& operator[](const size_t& index) { return Data[index]; }
		const T& operator[](const size_t& index) const { return Data[index]; }

		size_t push_back(const T& type) {
			size_t index = counter;
			Data[counter] = type;
			counter++;
			return index;
		}

		size_t push_back() {
			size_t index = counter;
			counter++;
			return index;
		}

		size_t size() const { return counter; }
		constexpr size_t allocated_size() const { return Size; }
		constexpr size_t byte_size() const { return Size * sizeof(T); }
	};
}