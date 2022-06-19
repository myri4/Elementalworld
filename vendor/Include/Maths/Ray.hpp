#pragma once

#include <glm/glm.hpp>

class Ray {
public:
	Ray() {}
	Ray(const glm::vec3& rayOrigin) : m_rayOrigin(rayOrigin){}

	void CalculateDirection(const float& Yaw, const float& Pitch) {
		float yaw = glm::radians(Yaw);
		float pitch = glm::radians(Pitch);

		glm::vec3 front;
		front.x = -glm::sin(yaw) * 2.f;
		front.y =  glm::tan(pitch) * 2.f;
		front.z = -glm::cos(yaw) * 2.f;
		m_rayDir = glm::normalize(front);
	}

	glm::vec3 m_rayOrigin = glm::vec3(0.f);
	glm::vec3 m_rayDir = glm::vec3(0.f);
};