#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <Utils/Window.hpp>

namespace wc{

	class Camera {
	public:
		// camera Attributes
		glm::vec3 Position = glm::vec3(0.f);
		glm::vec3 Front = glm::vec3(0.f, 0.f, -1.f);
		glm::vec3 Up = glm::vec3(0.f, 1.f, 0.f); // v

		//glm::quat m_orient;
		//void rotate(const float& angle, const glm::vec3& axis) { m_orient *= glm::angleAxis(angle, axis * m_orient); }
		//glm::mat4 view() const { return glm::translate(glm::mat4_cast(m_orient), Position); }

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
		float FOV = 90.f;

		// constructor with vectors
		Camera() {}

		// returns the view matrix calculated using Euler Angles and the LookAt Matrix
		glm::mat4 GetViewMatrix() const { return glm::lookAt(Position, Position + Front, Up); }

		void UpdateCameraAngles() {
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

			float theta = glm::tan(glm::radians(FOV) * 0.5f);
			float viewport_height = 2.f * theta;
			glm::vec2 windSize = window.GetSize();
			float viewport_width = windSize.x / windSize.y * viewport_height;

			glm::vec3 Right = normalize(cross(Front, up));  // u normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
			Up = normalize(cross(Right, Front));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
			horizontal = viewport_width * Right;
			vertical = viewport_height * Up;
			lower_left_corner = Position - horizontal * 0.5f - vertical * 0.5f + Front;
		}
	};
}