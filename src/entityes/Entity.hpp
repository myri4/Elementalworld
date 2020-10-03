#pragma once
#include <glm/glm.hpp>
#include <Utilitiess/NonCopyable.hpp>

namespace wc {
	class Entity : NonCopyable{		
	public:
		glm::vec3 Position;
		virtual ~Entity() = default;
		virtual void Update() = 0;

	};

}