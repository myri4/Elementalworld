#pragma once
#include "Entity.hpp"
#include "playerSpec/Ray.hpp"
#include <wclibs/pch.hpp>

namespace wc{
class Player : public Entity{
private:
	void Update() override {}
public:
	Camera camera;
	float Far = 1100.0f;

	glm::mat4 projection = glm::mat4(0.0f);
	Player() {}

	~Player() override{}
	void UpdatePlayerInput(const float& deltaTime) {
		float velocity = camera.MovementSpeed * deltaTime;
		if (wc::Keyboard::isButtonPressed(wc::Keyboard::Key::W)) { // Front
			camera.Position.x += glm::cos(glm::radians(camera.Yaw)) * velocity;
			camera.Position.z += glm::sin(glm::radians(camera.Yaw)) * velocity;
		}

		if (wc::Keyboard::isButtonPressed(wc::Keyboard::Key::S)) { // Back
			camera.Position.x -= glm::cos(glm::radians(camera.Yaw)) * velocity;
			camera.Position.z -= glm::sin(glm::radians(camera.Yaw)) * velocity;
		}
		if (wc::Keyboard::isButtonPressed(wc::Keyboard::Key::A)) { // Left
			camera.Position.x -= glm::cos(glm::radians(camera.Yaw + 90.0f)) * velocity;
			camera.Position.z -= glm::sin(glm::radians(camera.Yaw + 90.0f)) * velocity;
		}
		if (wc::Keyboard::isButtonPressed(wc::Keyboard::Key::D)) { // Right
			camera.Position.x -= glm::cos(glm::radians(camera.Yaw - 90.0f)) * velocity;
			camera.Position.z -= glm::sin(glm::radians(camera.Yaw - 90.0f)) * velocity;
		}
		if (wc::Keyboard::isButtonPressed(wc::Keyboard::Key::Space))  camera.Position.y += velocity;			  // Up
		if (wc::Keyboard::isButtonPressed(wc::Keyboard::Key::LShift)) camera.Position.y -= velocity;			  // Down
		if (wc::Keyboard::isButtonPressed(wc::Keyboard::Key::C)) { camera.Zoom = 10; camera.MouseSensitivity = 18; }
		else {
			camera.MouseSensitivity = 5;
			camera.Zoom = 90;
		}
	}
	void InitPlayer(const glm::vec3& Position) {
		camera.Position = Position;
	}
	void UpdatePlayer(const glm::vec2& windpos, const glm::vec2& windsize, const bool& CenterMouse, const float& deltaTime) {

		camera.UpdateCameraAngles(windpos, windsize, CenterMouse);
 
		projection = glm::perspective(glm::radians(camera.Zoom), windsize.x / windsize.y, 0.1f, Far);

		Position = camera.Position;
	}
	glm::mat4 GetView() {return camera.GetViewMatrix();}

};
class ServerPlayer : public Entity {
private:

public:

};
}
