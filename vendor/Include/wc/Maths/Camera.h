#pragma once

#include <glm/gtc/matrix_transform.hpp>

namespace wc{

	class Camera {
	public:
		// camera Attributes
		glm::vec3 Position = glm::vec3(0.f);
		glm::vec3 Front = glm::vec3(0.f, 0.f, -1.f);
		glm::vec3 Up = glm::vec3(0.f, 1.f, 0.f);

		// Ray tracing attributes
		glm::vec3 lower_left_corner = glm::vec3(0.f);
		glm::vec3 horizontal = glm::vec3(0.f);
		glm::vec3 vertical = glm::vec3(0.f);
		float distanceFromCamera = 0.f;
		// euler Angles
		float Yaw = 0.f;
		float Pitch = 0.f;
		float Roll = 90.f;
		// camera options
		float FOV = glm::radians(90.f);

		// constructor with vectors
		Camera() = default;

		// returns the view matrix calculated using Euler Angles and the LookAt Matrix
		glm::mat4 GetViewMatrix() const { return glm::lookAt(Position, Position + Front, Up); }

		void Update(const float& aspectRatio) {
			// update Front, Right and Up Vectors using the updated Euler angles
			// calculates the new Front vector from the Camera's (updated) Euler Angles
			float yaw = glm::radians(Yaw);
			float pitch = glm::radians(Pitch);

			Front.x = glm::cos(yaw) * glm::cos(pitch);
			Front.y = glm::sin(pitch);
			Front.z = glm::sin(yaw) * glm::cos(pitch);
			Front = glm::normalize(Front);

			//rotate(Yaw, glm::vec3(0.f, 1.f, 0.f));
			//rotate(Pitch, glm::vec3(1.f, 0.f, 0.f));
			//rotate(glm::radians(Roll), glm::vec3(0.f, 0.f, 1.f));

			glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
			//up.x = glm::cos(glm::radians(Roll));
			//up.y = glm::sin(glm::radians(Roll));
			//up = normalize(up);
			Position -= Front * distanceFromCamera;

			float theta = glm::tan(FOV * 0.5f);
			float viewport_height = 2.f * theta;
			float viewport_width = aspectRatio * viewport_height;

			glm::vec3 Right = normalize(cross(Front, up));  // u normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
			Up = normalize(cross(Right, Front));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
			horizontal = viewport_width * Right;
			vertical = viewport_height * Up;
			lower_left_corner = Position - horizontal * 0.5f - vertical * 0.5f + Front;
		}
	};
}