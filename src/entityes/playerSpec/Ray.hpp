#pragma once

#include <glm/glm.hpp>

class Ray {
public:
	Ray() {}
	Ray(const glm::vec3& startPos) : m_rayStart(startPos) {}
	~Ray() {}

	void Step(const float& Yaw, const float& Pitch, const float& step = 0.05f) {		
		m_rayEnd.x -= glm::cos(Yaw) * step;
		m_rayEnd.z -= glm::sin(Yaw) * step;
		m_rayEnd.y += glm::tan(Pitch) * step;
	}

	void SetStartPos(const glm::vec3& startPos) { m_rayStart = startPos; }

	const glm::vec3& getEnd() const { return m_rayEnd; }

	float getLength() const { return glm::distance(m_rayStart, m_rayEnd); }

private:
	glm::vec3 m_rayStart = glm::vec3(0.0f);
	glm::vec3 m_rayEnd = m_rayStart;
};