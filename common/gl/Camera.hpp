#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Utilitiess/Lua.hpp>
#include <Utilitiess/Mouse.hpp>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum class Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

namespace wc{
// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera{
public:
	// camera Attributes
	glm::vec3 Position = glm::vec3(0.0f);
	glm::vec3 Front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 Right = glm::vec3(0.0f);
	glm::vec3 WorldUp = glm::vec3(0.0f, 0.0f, 0.0f);
	// euler Angles
	float Yaw = 0.0f;
	float Pitch = 0.0f;
	// camera options
	float MovementSpeed = 2.5f;
	float MouseSensitivity = 5.f;
	float Zoom = 90.0f;

	// constructor with vectors
	Camera() {
	
	}
	void Create(const glm::vec3& position = glm::vec3(0.0f), const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f), const float& yaw = 0, const float& pitch = 0) {
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	// returns the view matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix(){return glm::lookAt(Position, Position + Front, Up);}

	// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void Move(const Camera_Movement& direction, const float& deltaTime){
		float velocity = MovementSpeed * deltaTime;
		if (direction == Camera_Movement::FORWARD)  Position += Front * velocity;
		if (direction == Camera_Movement::BACKWARD) Position -= Front * velocity;
		if (direction == Camera_Movement::LEFT)     Position -= Right * velocity;
		if (direction == Camera_Movement::RIGHT)	Position += Right * velocity;
		if (direction == Camera_Movement::UP)	    Position.y += velocity;
		if (direction == Camera_Movement::DOWN)	    Position.y -= velocity;
	}

	void UpdateCameraAngles(const glm::vec2& windowpos, const bool& centerMouse = true, const bool& invertMouse = false) {

		int32_t xt, yt;
		
		glm::vec2 pos = wc::Mouse::GetMousePos();

		xt = windowpos.x + 400;
		yt = windowpos.y + 300;

		
		if (invertMouse) Yaw += (xt - pos.x) / MouseSensitivity;
		else Yaw -= (xt - pos.x) / MouseSensitivity;

		Pitch += (yt - pos.y) / MouseSensitivity;

		// make sure that when pitch is out of bounds, screen doesn't get flipped
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;

			if (Yaw > 360.0f)
				Yaw = 0.0f;
			if (Yaw < -360.0f)
				Yaw = 0.0f;

		// update Front, Right and Up Vectors using the updated Euler angles
			if (centerMouse) {
				updateCameraVectors();
				wc::Mouse::SetMousePosition(xt, yt);
			}
	}
private:
	// calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors(){
		// calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);
		// also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
	}
};
}