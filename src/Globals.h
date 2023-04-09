#pragma once
#include <glm/glm.hpp>
#include <wc/Utils/Window.h>

namespace wc {
	inline Window window;

	constexpr uint32_t RenderDistance = 16;

	//@TODO: Convert to state machine
	enum MenuMode : uint32_t { GAME, INVENTORY, MAINMENU, SETTINGS, MULTIPLAYER, ESCMENU, WORLD_SELECTION, WORLD_CREATION };
	inline MenuMode menuMode = MenuMode::MAINMENU;
	inline MenuMode previousMode = menuMode;

	#undef ChangeMenu
	void ChangeMenu(MenuMode newMode) {
		previousMode = menuMode;
		menuMode = newMode;

		if (menuMode == GAME) {
			window.setCursorPos(window.GetSize() / 2);
			window.SetCursorMode(GLFW_CURSOR_DISABLED);
		}
		else 
			window.SetCursorMode(GLFW_CURSOR_NORMAL);
	}

	void ChangeBack() {
		//ChangeMenu(previousMode);
		menuMode = previousMode;
	}

	uint32_t convertColor(const glm::vec4& color) {
		int32_t r = int32_t(color.r * 255.f);
		int32_t g = int32_t(color.g * 255.f);
		int32_t b = int32_t(color.b * 255.f);
		int32_t a = int32_t(color.a * 255.f);
		return a << 24 | b << 16 | g << 8 | r;
	}

	ImVec2 scaleRes(const ImVec2& position) {
		const ImVec2 malenRes = ImVec2(1920, 1080);
		ImVec2 windowRes = ImVec2(window.GetSize().x, window.GetSize().y);
		ImVec2 finalPos;
		finalPos.x = position.x / malenRes.x * windowRes.x;
		finalPos.y = position.y / malenRes.y * windowRes.y;
		return finalPos;
	}

	glm::vec4 convertColor(uint32_t color) {
		const float c = 1.f / 255.f;
		glm::vec4 Color;
		Color.r = float((uint32_t)(color & uint32_t(0x000000ff))) * c;
		Color.g = float((uint32_t)(color & uint32_t(0x0000ff00)) >> 8) * c;
		Color.b = float((uint32_t)(color & uint32_t(0x00ff0000)) >> 16) * c;
		Color.a = float((uint32_t)(color & uint32_t(0xff000000)) >> 24) * c;

		return Color;
	}	
	int to1D(int x, int y, int z, uint32_t size) { return (z * size * size) + (y * size) + x; }

	glm::ivec3 to3D(int idx, uint32_t size) {
		int i = idx;
		int z = i / (size * size);
		i -= (z * size * size);
		int y = i / size;
		int x = i % size;
		return glm::ivec3(x, y, z);
	}
}