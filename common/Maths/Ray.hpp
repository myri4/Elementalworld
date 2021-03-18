#ifndef RAY_HPP
#define RAY_HPP

#include <glm/glm.hpp>

class Ray {
public:
	Ray() {}
	Ray(const glm::vec3& startPos) : m_rayStart(startPos){}

	void Step(const float& Yaw, const float& Pitch, const float& step = 0.5f) {
		float yaw = glm::radians(Yaw);
		float pitch = glm::radians(Pitch);

		glm::vec3 front;
		front.x = glm::cos(yaw) * glm::cos(pitch);
		front.y = glm::tan(pitch);
		front.z = glm::sin(yaw) * glm::cos(pitch);
		m_rayEnd += glm::normalize(front);
	}

	glm::vec3 getEnd() const { return m_rayEnd; }

	float getLength() const { return glm::distance(m_rayStart, m_rayEnd); }

	glm::vec3 m_rayStart = glm::vec3(0.0f);
	glm::vec3 m_rayEnd = m_rayStart;
};
#endif