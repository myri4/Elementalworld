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
	BlockID ItemHolding = 1;

	glm::mat4 projection = glm::mat4(0.0f);
	Player() {}

	~Player() override{}

	void UpdatePlayerInput(const float& deltaTime) {
		float velocity = camera.MovementSpeed * deltaTime;
		if (Keyboard::isButtonPressed(Keyboard::Key::W)) { // Front
			camera.Position.x += glm::cos(glm::radians(camera.Yaw)) * velocity;
			camera.Position.z += glm::sin(glm::radians(camera.Yaw)) * velocity;
		}

		if (Keyboard::isButtonPressed(Keyboard::Key::S)) { // Back
			camera.Position.x -= glm::cos(glm::radians(camera.Yaw)) * velocity;
			camera.Position.z -= glm::sin(glm::radians(camera.Yaw)) * velocity;
		}
		if (Keyboard::isButtonPressed(Keyboard::Key::A)) { // Left
			camera.Position.x -= glm::cos(glm::radians(camera.Yaw + 90.0f)) * velocity;
			camera.Position.z -= glm::sin(glm::radians(camera.Yaw + 90.0f)) * velocity;
		}
		if (Keyboard::isButtonPressed(Keyboard::Key::D)) { // Right
			camera.Position.x -= glm::cos(glm::radians(camera.Yaw - 90.0f)) * velocity;
			camera.Position.z -= glm::sin(glm::radians(camera.Yaw - 90.0f)) * velocity;
		}
		if (Keyboard::isButtonPressed(Keyboard::Key::Space))  camera.Position.y += velocity;			  // Up
		if (Keyboard::isButtonPressed(Keyboard::Key::LShift)) camera.Position.y -= velocity;			  // Down
		if (Keyboard::isButtonPressed(Keyboard::Key::C)) { camera.Zoom = 10; camera.MouseSensitivity = 18; }
		else {
			camera.MouseSensitivity = 5;
			camera.Zoom = 90;
		}

		if (Keyboard::isButtonPressed(Keyboard::Key::Num1)) ItemHolding = 1;
		if (Keyboard::isButtonPressed(Keyboard::Key::Num2)) ItemHolding = 2;
		if (Keyboard::isButtonPressed(Keyboard::Key::Num2)) ItemHolding = 2;
		if (Keyboard::isButtonPressed(Keyboard::Key::Num3)) ItemHolding = 3;
		if (Keyboard::isButtonPressed(Keyboard::Key::Num4)) ItemHolding = 4;
		if (Keyboard::isButtonPressed(Keyboard::Key::Num5)) ItemHolding = 5;
		if (Keyboard::isButtonPressed(Keyboard::Key::Num6)) ItemHolding = 6;
		if (Keyboard::isButtonPressed(Keyboard::Key::Num7)) ItemHolding = 7;
		if (Keyboard::isButtonPressed(Keyboard::Key::Num8)) ItemHolding = 8;
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
