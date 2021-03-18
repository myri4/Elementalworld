#ifndef PLAYER_HPP
#define PLAYER_HPP
#include "Entity.hpp"
#include <Maths/Ray.hpp>
#include <Utils/Mouse.hpp>

namespace wc{
class Player{
public:	
	float MovementSpeed = 4;
	BlockID ItemHolding = 1;
	glm::vec3 Position = glm::vec3(0.f);
	glm::vec3 Size = glm::vec3(1.0f);
	bool m_isOnGround;
	glm::vec3 velocity;
	glm::vec3 m_acceleration;

	Player() {}
};

class PlayerDescription {
public:
	uint32_t nUniqueID = 0;
	glm::vec3 Position = glm::vec3(0.f);
};
}
#endif