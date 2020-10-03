#pragma once
#include "Entity.hpp"
#include <wclibs/pch.hpp>

namespace wc{
class Player : public Entity{
private:
public:
	Camera camera;

	glm::mat4 view;
	glm::mat4 projection;
	Player() {

	}

	~Player() override{

	}
	void Update() override {

	}
	void UpdatePlayerInput(float deltaTime) {
		if (ew::Keyboard::isButtonPressed(ew::Keyboard::Key::W))      camera.Move(FORWARD, deltaTime);
		if (ew::Keyboard::isButtonPressed(ew::Keyboard::Key::S))      camera.Move(BACKWARD, deltaTime);
		if (ew::Keyboard::isButtonPressed(ew::Keyboard::Key::A))      camera.Move(LEFT, deltaTime);
		if (ew::Keyboard::isButtonPressed(ew::Keyboard::Key::D))      camera.Move(RIGHT, deltaTime);
		if (ew::Keyboard::isButtonPressed(ew::Keyboard::Key::Space))  camera.Move(UP, deltaTime);
		if (ew::Keyboard::isButtonPressed(ew::Keyboard::Key::LShift)) camera.Move(DOWN, deltaTime);
		if (ew::Keyboard::isButtonPressed(ew::Keyboard::Key::C)) { camera.Zoom = 10; camera.MouseSensitivity = 18; }
		else {
			camera.MouseSensitivity = 5;
			camera.Zoom = 90;
		}
	}
	void InitPlayer(glm::vec3 Position) {
		camera.Create(glm::vec3(Position));
	}
	void UpdatePlayer(glm::vec2 windsize, bool CenterMouse, float deltaTime) {

		camera.UpdateCameraAngles(windsize, CenterMouse);

		view = camera.GetViewMatrix(); 
		projection = glm::perspective(glm::radians(camera.Zoom), windsize.x / windsize.y, 0.1f, 100.0f);

	}

};
class ServerPlayer : public Entity {
private:

public:

};
}
