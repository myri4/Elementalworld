#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace wc{
// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera{
public:
	// camera Attributes
	glm::vec3 Position = glm::vec3(0.0f);
	glm::vec3 Front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 Right = glm::vec3(0.0f);
	// euler Angles
	float Yaw = 0.0f;
	float Pitch = 0.0f;
	// camera options
	float FOV = 90.0f;

	// constructor with vectors
	Camera() {}

	// returns the view matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix() const { return glm::lookAt(Position, Position + Front, Up); }	

	void UpdateCameraAngles() {
		// update Front, Right and Up Vectors using the updated Euler angles
		// calculates the new Front vector from the Camera's (updated) Euler Angles
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);
		
		// also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, glm::vec3(0.0f, 1.0f, 0.0f)));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));			
	}
};
}
#endif