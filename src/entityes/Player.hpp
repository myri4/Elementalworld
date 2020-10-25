#pragma once
#include "Entity.hpp"
#include "playerSpec/Ray.hpp"
#include <wclibs/pch.hpp>

namespace wc{
class Player : public Entity{
private:
public:
	Camera camera;

	glm::mat4 projection = glm::mat4(0.0f);
	Player() {

	}

	~Player() override{

	}
	void Update() override {

	}
	void UpdatePlayerInput(const float& deltaTime) {
		if (wc::Keyboard::isButtonPressed(wc::Keyboard::Key::W))      camera.Move(Camera_Movement::FORWARD, deltaTime);
		if (wc::Keyboard::isButtonPressed(wc::Keyboard::Key::S))      camera.Move(Camera_Movement::BACKWARD, deltaTime);
		if (wc::Keyboard::isButtonPressed(wc::Keyboard::Key::A))      camera.Move(Camera_Movement::LEFT, deltaTime);
		if (wc::Keyboard::isButtonPressed(wc::Keyboard::Key::D))      camera.Move(Camera_Movement::RIGHT, deltaTime);
		if (wc::Keyboard::isButtonPressed(wc::Keyboard::Key::Space))  camera.Move(Camera_Movement::UP, deltaTime);
		if (wc::Keyboard::isButtonPressed(wc::Keyboard::Key::LShift)) camera.Move(Camera_Movement::DOWN, deltaTime);
		if (wc::Keyboard::isButtonPressed(wc::Keyboard::Key::C)) { camera.Zoom = 10; camera.MouseSensitivity = 18; }
		else {
			camera.MouseSensitivity = 5;
			camera.Zoom = 90;
		}
		Ray ray(camera);

		for (; ray.getLength() < 8; ray.Step()) {

		}
	}
	void InitPlayer(const glm::vec3& Position) {
		camera.Create(glm::vec3(Position));
	}
	void UpdatePlayer(const glm::vec2& windsize, const bool& CenterMouse, const float& deltaTime) {

		camera.UpdateCameraAngles(windsize, CenterMouse);
 
		projection = glm::perspective(glm::radians(camera.Zoom), windsize.x / windsize.y, 0.1f, 100.0f);

		Position = camera.Position;
	}
	glm::mat4 GetView() {return camera.GetViewMatrix();}


};
class ServerPlayer : public Entity {
private:

public:

};
}
