#ifndef PLAYER_HPP
#define PLAYER_HPP
#include "Entity.hpp"
#include <Maths/Ray.hpp>
#include <Utils/Mouse.hpp>
#include "../Game Mechanics/Inventory.hpp"

namespace wc{
class Player{
public:	
	float MovementSpeed = 4;
	BlockID ItemHolding = 1;
	glm::vec3 Position = glm::vec3(0.f);
	glm::vec3 Size = glm::vec3(1.0f);
	bool m_isOnGround = false;
	bool flying = true;
	glm::vec3 velocity;
	Inventory<9, 4> inventory;

	Player() {}
};

class PlayerDescription {
public:
	uint32_t nUniqueID = 0;
	glm::vec3 Position = glm::vec3(0.f);
};
}
#endif