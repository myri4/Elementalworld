#pragma once

#include <thread>
#include <memory>

#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <ostream>
#include <unordered_map>
#include <sstream>
#include <array>

#define GLM_FORCE_PURE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//Internet connection
#include <net/wc_net.hpp>

//Util
#include <Utils/Log.hpp>
#include <Utils/Time.hpp>
#include <Utils/Random.hpp>

namespace wc {
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