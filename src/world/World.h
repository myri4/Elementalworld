#pragma once
#include <map>
#include "Chunk.h"

namespace wc {
	class World
	{
	public:
		std::map<glm::vec3, Chunk> worldMap;

	private:

	};

}