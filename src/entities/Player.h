#pragma once
#include "../Game Mechanics/Inventory.h"

namespace wc {
	const uint32_t inventorySizeX = 9;
	const uint32_t inventorySizeY = 4;

	float minFallDistance = 3.f;

	class Player {
	public:
		std::string name = "Player 1";
		float MovementSpeed = 4.f;
		glm::vec2 rotation = glm::vec2(0.f); // x: Yaw y: Pitch z: Roll
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
		float health = 10.f;
		Inventory inventory;

		Player() {
			m_isOnGround = false;
			flying = true;
			wasOnGround = false;
			collision = false;
		}

		void Serialize(const std::string& location) const {
			YAML::Node config;
			config["MovementSpeed"] = MovementSpeed;
			config["rotation"] = rotation;
			config["currentSlot"] = (uint32_t)currentSlot;
			config["position"] = Position;
			config["flying"] = (uint32_t)flying;
			config["collision"] = (uint32_t)collision;
			config["velocity"] = velocity;
			config["acceleration"] = acceleration;
			config["health"] = health;
			YAMLUtils::saveFile(location + name + ".ec", config);
		}

		void Deserialize(const std::string& location) {
			std::string searchLoc = location + name + ".ec";
			if (!std::filesystem::exists(searchLoc)) Serialize(location);
			else {
				YAML::Node config = YAML::LoadFile(searchLoc);
				if (config["MovementSpeed"]) MovementSpeed = config["MovementSpeed"].as<float>();
				if (config["rotation"])      rotation = config["rotation"].as<glm::vec2>();
				if (config["currentSlot"])   currentSlot = config["currentSlot"].as<uint32_t>();
				if (config["position"])      Position = config["position"].as<glm::vec3>();
				if (config["flying"])        flying = config["flying"].as<uint32_t>();
				if (config["collision"])     collision = config["collision"].as<uint32_t>();
				if (config["velocity"])      velocity = config["velocity"].as<glm::vec3>();
				if (config["acceleration"])  acceleration = config["acceleration"].as<glm::vec3>();
				if (config["health"])        health = config["health"].as<float>();
			}
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