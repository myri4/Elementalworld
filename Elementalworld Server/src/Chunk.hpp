#pragma once
#include <glm/glm.hpp>

//OpenGL Memory Buffer Variables
//@Todo try with size_t 
const uint16_t chunkSize = 16;

typedef uint8_t BlockID; // This represents the chunk id in the chunk array
typedef uint16_t ChunkID; // This represents the chunk id in the chunk array

namespace wc {	

	class Chunk {
	public: // Variables
		glm::ivec3 position = glm::ivec3(0);
		BlockID data[chunkSize][chunkSize][chunkSize] = { 0 };

		Chunk() {}
	};
}