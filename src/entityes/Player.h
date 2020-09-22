#pragma once
#include "Entity.hpp"
#include <wclibs/wclibspch.h>

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
	void UpdatePlayerInput(float deltaTime, sf::RenderWindow &window) {
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))		  camera.Move(FORWARD, deltaTime);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))      camera.Move(BACKWARD, deltaTime);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))      camera.Move(LEFT, deltaTime);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))      camera.Move(RIGHT, deltaTime);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))  camera.Move(UP, deltaTime);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) camera.Move(DOWN, deltaTime);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::C)) { camera.Zoom = 10; camera.MouseSensitivity = 18; }
		else {
			camera.MouseSensitivity = 5;
			camera.Zoom = 90;
		}

		camera.UpdateCameraAngles(window);
	}
	void InitPlayer() {
		camera.Create(glm::vec3(Position));
	}
	void UpdatePlayer(sf::RenderWindow &window, bool CenterMouse, float deltaTime) {

		camera.UpdateCameraAngles(window, CenterMouse);

		view = camera.GetViewMatrix();
		projection = glm::perspective(glm::radians(camera.Zoom), (float)window.getSize().x / (float)window.getSize().y, 0.1f, 100.0f);

		UpdatePlayerInput(deltaTime, window);
	}

};
class ServerPlayer : public Entity {
private:

public:

};
}
