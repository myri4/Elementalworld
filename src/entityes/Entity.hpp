#pragma once
#include <glm/glm.hpp>

namespace wc {
	class Entity{
	protected:
		
	public:
		glm::vec3 Position;
		Entity() {

		}
		virtual ~Entity() = 0;


	};

}