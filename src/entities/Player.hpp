#ifndef PLAYER_HPP
#define PLAYER_HPP
#include "../Game Mechanics/Inventory.hpp"

namespace wc{
class Player{
public:	
	float MovementSpeed = 4;
	glm::vec2 rotation = glm::vec2(0.f);
	BlockID ItemHolding = 1;
	glm::vec3 Position = glm::vec3(0.f);
	glm::vec3 Size = glm::vec3(1.f);
	bool m_isOnGround = false;
	bool flying = true;
	glm::vec3 velocity = glm::vec3(0.f);
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