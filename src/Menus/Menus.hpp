#pragma once
#include <GUI/Button.hpp>
#include <GUI/Textbox.hpp>
#include <wc/pch.hpp>
#include <Utils/Window.hpp>

namespace wc {

	enum class MenuMode { GAME, INVENTORY, MAINMENU, SETTINGS, MULTIPLAYER, ESCMENU };
	MenuMode mode = MenuMode::MAINMENU; // @TODO: Remove it from here and make a file that needs to be include everywhere

	class MainMenu {
		TextButton singlePlayer;
		TextButton multiPlayer;
		TextButton settings;
	public:
		void OnCreate(const Font& font, const float& scale) {
			singlePlayer.text = "Singleplayer";
			singlePlayer.CenterText(font, scale);

			singlePlayer.position = { 0,0 };
			singlePlayer.size = { 500,100 };

			multiPlayer.text = "Multiplayer";
			multiPlayer.CenterText(font, scale);

			multiPlayer.position = { 0,100 };
			multiPlayer.size = { 500,100 };

			settings.text = "Settings";
			settings.CenterText(font, scale);

			settings.position = { 0,200 };
			settings.size = { 500,100 };
		}

		void OnInput() {
			if (singlePlayer.isMouseOver() && Mouse::getMouse(GLFW_MOUSE_BUTTON_LEFT)) {
				int16_t xt, yt;
				glm::ivec2 windSize = window.GetSize();
				xt = windSize.x / 2;
				yt = windSize.y / 2;
				Mouse::SetMousePosition(xt, yt);
				mode = MenuMode::GAME; 
			}
			//else if (settings.isMouseOver() && Mouse::getMouse(GLFW_MOUSE_BUTTON_LEFT)) mode = MenuMode::SETTINGS;
			else if (multiPlayer.isMouseOver() && Mouse::getMouse(GLFW_MOUSE_BUTTON_LEFT)) mode = MenuMode::MULTIPLAYER;
		}

		void OnUpdate(const Font& font, const float& scale) {
			singlePlayer.Render(font, scale);
			multiPlayer.Render(font, scale);
			settings.Render(font, scale);
		}
	};

	bool bReloadChunkShader = false;
	bool bReloadModelShader = false;

	class EscMenu {
		TextButton resume;
		TextButton mainMenu;
		TextButton quitGame;
		TextButton reloadChunkShader;
		TextButton reloadModelShader;
	public:
		void OnCreate(const Font& font, const float& scale) {
			resume.text = "Resume";
			resume.CenterText(font, scale);

			resume.position = { 0,0 };
			resume.size = { 500,100 };

			mainMenu.text = "Main menu";
			mainMenu.CenterText(font, scale);

			mainMenu.position = { 0,100 };
			mainMenu.size = { 500,100 };

			quitGame.text = "Quit game";
			quitGame.CenterText(font, scale);

			quitGame.position = { 0,200 };
			quitGame.size = { 500,100 };

			reloadChunkShader.text = "Reload chunk shader";
			reloadChunkShader.CenterText(font, scale);

			reloadChunkShader.position = { 500,0 };
			reloadChunkShader.size = { 500,100 };

			reloadModelShader.text = "Reload model shader";
			reloadModelShader.CenterText(font, scale);

			reloadModelShader.position = { 500,100 };
			reloadModelShader.size = { 500,100 };
		}

		void OnInput() {
			if (resume.isMouseOver() && Mouse::getMouse(GLFW_MOUSE_BUTTON_LEFT)) {
				int16_t xt, yt;
				glm::ivec2 windSize = window.GetSize();
				xt = windSize.x / 2;
				yt = windSize.y / 2;
				Mouse::SetMousePosition(xt, yt);
				mode = MenuMode::GAME;
			}
			else if (mainMenu.isMouseOver() && Mouse::getMouse(GLFW_MOUSE_BUTTON_LEFT)) mode = MenuMode::MAINMENU;
			else if (quitGame.isMouseOver() && Mouse::getMouse(GLFW_MOUSE_BUTTON_LEFT)) window.close();
			else if (reloadChunkShader.isMouseOver() && Mouse::getMouse(GLFW_MOUSE_BUTTON_LEFT)) bReloadChunkShader = true;
			else if (reloadModelShader.isMouseOver() && Mouse::getMouse(GLFW_MOUSE_BUTTON_LEFT)) bReloadModelShader = true;
		}

		void OnUpdate(const Font& font, const float& scale) {
			resume.Render(font, scale);
			mainMenu.Render(font, scale);
			quitGame.Render(font, scale);
			reloadChunkShader.Render(font, scale);
			reloadModelShader.Render(font, scale);
		}
	};

	class MultiplayerMenu {
		TextButton connect;
		TextButton ipButton;
		TextButton playerNameButton;
	public:
		bool shouldConnect = false;
		Textbox ipTextbox;
		Textbox playerName;
		void OnCreate(const Font& font, const float& scale) {
			connect.text = "Connect";
			connect.CenterText(font, scale);

			connect.position = { 510,0 };
			connect.size = { 200,100 };


			ipButton.text = ipTextbox.text;
			ipButton.CenterText(font, scale);

			ipButton.position = { 0,0 };
			ipButton.size = { 500,100 };


			playerNameButton.text = playerName.text;
			playerNameButton.CenterText(font, scale);

			playerNameButton.position = { 0,200 };
			playerNameButton.size = { 500,100 };
		}

		void OnInput() {
			if (connect.isMouseOver() && Mouse::getMouse(GLFW_MOUSE_BUTTON_LEFT)) {
				int16_t xt, yt;
				glm::ivec2 windSize = window.GetSize();
				xt = windSize.x / 2;
				yt = windSize.y / 2;
				Mouse::SetMousePosition(xt, yt);
				mode = MenuMode::GAME;
				shouldConnect = true;
			}
			if (playerNameButton.isMouseOver() && Mouse::getMouse(GLFW_MOUSE_BUTTON_LEFT)) { 
				playerName.isSelected = !playerName.isSelected;
				ipTextbox.isSelected = false;
			}
			if (ipButton.isMouseOver() && Mouse::getMouse(GLFW_MOUSE_BUTTON_LEFT)) {
				ipTextbox.isSelected = !ipTextbox.isSelected;
				playerName.isSelected = false;
			}

			ipTextbox.update();
			playerName.update();
		}

		void OnUpdate(const Font& font, const float& scale) {
			ipButton.text = ipTextbox.text;
			ipButton.CenterText(font, scale);
			ipButton.textSize.x = 0;

			playerNameButton.text = playerName.text;
			playerNameButton.CenterText(font, scale);
			playerNameButton.textSize.x = 0;

			connect.Render(font, scale);
			ipButton.Render(font, scale);
			playerNameButton.Render(font, scale);
		}
	};
}