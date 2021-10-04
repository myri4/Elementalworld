#pragma once

#include <glm/glm.hpp>

class Particle {
public:
	glm::vec3 Position;
	glm::vec3 Velocity;
	float lifeLenght;
	float rotation;
	float scale;

	float elapsedTime;
private:
};