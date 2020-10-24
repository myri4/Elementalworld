#pragma once

#include <gl/Camera.hpp>

class Ray {
public:
	Ray() {}
	Ray(const wc::Camera& camera) : m_rayStart(camera.Position), m_rayEnd(camera.Position), Yaw(camera.Yaw), Pitch(camera.Pitch) {}
	~Ray() {}

	void Step(float step = 0.5f) {
		auto& p = m_rayEnd;

		p.x -= glm::cos(Yaw) * step;
		p.z -= glm::sin(Yaw) * step;
		p.y -= glm::tan(Pitch) * step;
	}

	const glm::vec3& getEnd() {
		return m_rayEnd;
	}

	float getLength() const {
		return glm::distance(m_rayStart, m_rayEnd);
	}

private:
	glm::vec3 m_rayStart = glm::vec3(0.0f);
	glm::vec3 m_rayEnd = glm::vec3(0.0f);

	float Yaw = 0.0f;
	float Pitch = 0.0f;

};