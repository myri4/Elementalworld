#pragma once
#include "../Game Mechanics/Inventory.hpp"

namespace wc {
	const uint32_t inventorySizeX = 9;
	const uint32_t inventorySizeY = 4;

	float minFallDistance = 3.f;

	class Player {
	public:
		std::string name = "Player 1";
		float MovementSpeed = 4.f;
		glm::vec2 rotation = glm::vec2(0.f);
		uint8_t currentSlot = 0;
		glm::vec3 Position = glm::vec3(0.f);
		glm::vec3 Size = glm::vec3(0.48f, 0.8f, 0.48f);
		bool m_isOnGround : 1;
		bool wasOnGround : 1;
		bool wasFalling : 1;
		float startOfFall = 0.f;
		bool flying : 1;
		bool collision : 1;
		glm::vec3 velocity = glm::vec3(0.f);
		glm::vec3 acceleration = glm::vec3(0.f);
		//InventoryMenu<inventorySizeX, inventorySizeY, inventorySizeX * inventorySizeY> inventory;
		//Crafter<2> crafting;
		float health = 10.f;

		Player() {
			m_isOnGround = false;
			flying = true;
			wasOnGround = false;
			collision = false;
		}

		bool isFalling() {
			return !m_isOnGround && velocity.y < 0.f;
		}
	};

	class PlayerDescription {
	public:
		uint32_t nUniqueID = 0;
		std::string name = "Player 1";
		glm::vec2 rotation = glm::vec2(0.f);
		uint8_t currentSlot = 0;
		glm::vec3 Position = glm::vec3(0.f);
		glm::vec3 velocity = glm::vec3(0.f);
		glm::vec3 acceleration = glm::vec3(0.f);
		float health = 10.f;
	};
}