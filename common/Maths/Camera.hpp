#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Utils/Window.hpp>
#include "../Utils/Mouse.hpp"

namespace wc{

class Camera {
public:
	// camera Attributes
	glm::vec3 Position = glm::vec3(0.f);
	glm::vec3 Front = glm::vec3(0.f, 0.f, -1.f);
	glm::vec3 Up = glm::vec3(0.f, 1.f, 0.f);
	glm::vec3 Right = glm::vec3(0.f);
	float distanceFromCenter = 5.f;
	// euler Angles
	float Yaw = 0.f;
	float Pitch = 0.f;
	// camera options
	float FOV = 90.f;

	void center(const glm::vec3& center, const float& yaw) {
		if (mouseScrolled) {
			distanceFromCenter -= (scrollY * 0.1f);
			Pitch -= (scrollY * 0.1f);
		}
		float horizontalDistance = distanceFromCenter * glm::cos(glm::radians(Pitch));
		float verticleDistance = distanceFromCenter * glm::sin(glm::radians(Pitch));

		float theta = glm::radians(yaw);
		float offsetX = horizontalDistance * glm::sin(theta);
		float offsetZ = horizontalDistance * glm::cos(theta);
		Position.x = center.x - offsetX;
		Position.z = center.z - offsetZ;
		Position.y = center.y + verticleDistance;

		Yaw = 90.f - yaw;
	}

	// constructor with vectors
	Camera() {}

	// returns the view matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix() const { return glm::lookAt(Position, Position + Front, Up); }

	void UpdateCameraAngles() {
		// update Front, Right and Up Vectors using the updated Euler angles
		// calculates the new Front vector from the Camera's (updated) Euler Angles
		float yaw = glm::radians(Yaw);
		float pitch = glm::radians(Pitch);
		glm::vec3 front;
		front.x = glm::cos(yaw) * glm::cos(pitch);
		front.y = glm::sin(pitch);
		front.z = glm::sin(yaw) * glm::cos(pitch);
		Front = glm::normalize(front);

		// also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, glm::vec3(0.f, 1.f, 0.f)));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
	}
};
}
#endif