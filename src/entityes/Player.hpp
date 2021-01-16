#pragma once
#include "Entity.hpp"
#include "playerSpec/Ray.hpp"
#include <wclibs/pch.hpp>
#include <Utils/Mouse.hpp>

namespace wc{
class Player : public Entity{
private:
	void Update() override {}

	float MouseSensitivity = 5;
	float MovementSpeed = 4;
	float Far = chunkSize * 15; // 1100
public:
	Camera camera;
	BlockID ItemHolding = 1;

	glm::mat4 projection = glm::mat4(0.0f);
	Player() {}

	~Player() override{}

	void UpdatePlayerInput(const float& deltaTime) {
		float velocity = MovementSpeed * deltaTime;
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
		if (Keyboard::isButtonPressed(Keyboard::Key::C)) { camera.FOV = 10; MouseSensitivity = 18; }
		else {
			MouseSensitivity = 5;
			camera.FOV = 90;
		}

		if (Keyboard::isButtonPressed(Keyboard::Key::Num1)) ItemHolding = 1;
		if (Keyboard::isButtonPressed(Keyboard::Key::Num2)) ItemHolding = 2;
		if (Keyboard::isButtonPressed(Keyboard::Key::Num2)) ItemHolding = 2;
		if (Keyboard::isButtonPressed(Keyboard::Key::Num3)) ItemHolding = 3;
		if (Keyboard::isButtonPressed(Keyboard::Key::Num4)) ItemHolding = 4;
		if (Keyboard::isButtonPressed(Keyboard::Key::Num5)) ItemHolding = 5;
		if (Keyboard::isButtonPressed(Keyboard::Key::Num6)) ItemHolding = 6;
		if (Keyboard::isButtonPressed(Keyboard::Key::Num7)) ItemHolding = 7;
		if (Keyboard::isButtonPressed(Keyboard::Key::Num8)) ItemHolding = 11;
	}

	void InitPlayer(const glm::vec3& Position) {
		camera.Position = Position;
	}

	void UpdatePlayer(const glm::vec2& windpos, const glm::vec2& windsize, const bool& CenterMouse, const float& deltaTime) {

		int16_t xt, yt;
		
		glm::vec2 pos = wc::Mouse::GetMousePos();
		
		xt = windpos.x + windsize.x / 2;
		yt = windpos.y + windsize.y / 2;
		
		bool invertMouse = false;
		if (invertMouse) camera.Yaw += (xt - pos.x) / MouseSensitivity;
		else camera.Yaw -= (xt - pos.x) / MouseSensitivity;

		camera.Pitch += (yt - pos.y) / MouseSensitivity;
		
		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (camera.Pitch > 89.0f) camera.Pitch = 89.0f;
		if (camera.Pitch < -89.0f)camera.Pitch = -89.0f;
		
		if (camera.Yaw > 360.0f) camera.Yaw = 0.0f;
		if (camera.Yaw < 0.0f)	 camera.Yaw = 360.0f;

		if (CenterMouse) {
			camera.UpdateCameraAngles();
			wc::Mouse::SetMousePosition(xt, yt);
		}
 
		projection = glm::perspective(glm::radians(camera.FOV), windsize.x / windsize.y, 0.1f, Far);

		Position = camera.Position;
	}

	glm::mat4 GetView() {return camera.GetViewMatrix();}

};
}
