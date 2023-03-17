#pragma once
#include <glm/glm.hpp>
#include <wc/vk/Buffer.h>
#include <wc/vk/Pipeline.h>
#include <wc/Utils/Window.h>

const uint8_t WC_MODEL_BIT = 0x1;
const uint8_t WC_CULL_BIT = 0x2;

namespace wc {
	Window window; // @TODO: [myri4] Move this to window manager

	//@Todo try with size_t 
	const uint16_t chunkSize = 16;
	const uint32_t chunkVolume = chunkSize * chunkSize * chunkSize;

	const uint32_t MaxFaceCount = chunkSize * chunkSize * 4;
	const uint32_t MaxVertexCount = MaxFaceCount * 4;
	const uint32_t MaxIndexCount = MaxFaceCount * 6;

	static const uint32_t RenderDistance = 16;

	using ChunkID = uint16_t;
	using BlockID = uint8_t;
	using MaterialID = uint8_t;
	using MeshID = uint32_t;

	enum ConnectionType : uint8_t {
		CONNECT_DEFAULT, FLUID_CONNECT, NO_CONNECT,
		SLAB_DOWN, SLAB_UP, SLAB_LEFT, SLAB_RIGHT, SLAB_FRONT, SLAB_BACK,
		CANT_CONNECT, AIR, CUSTOM_MODEL, NON_EXISTENT
	};
	enum class BlockTexture : uint8_t { RIGHT, LEFT, TOP, BOTTOM, FRONT, BACK };

	//@TODO: Convert to state machine
	enum MenuMode : uint32_t { GAME, INVENTORY, MAINMENU, SETTINGS, MULTIPLAYER, ESCMENU, WORLD_SELECTION, WORLD_CREATION };
	MenuMode menuMode = MenuMode::MAINMENU;
	MenuMode previousMode = menuMode;

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

	glm::vec4 convertColor(uint32_t color) {
		const float c = 1.f / 255.f;
		glm::vec4 Color;
		Color.r = float((uint32_t)(color & uint32_t(0x000000ff))) * c;
		Color.g = float((uint32_t)(color & uint32_t(0x0000ff00)) >> 8) * c;
		Color.b = float((uint32_t)(color & uint32_t(0x00ff0000)) >> 16) * c;
		Color.a = float((uint32_t)(color & uint32_t(0xff000000)) >> 24) * c;

		return Color;
	}

	struct Vertex {
		glm::vec3 Position = { 0,0,0 };
		uint32_t materialID = 0;
		glm::vec2 TexCoords = { 0,0 };
		uint32_t _pad2[2] = { 0 };
		glm::vec3 Normal = { 0,0,0 };
		uint32_t _pad3 = 0;
		Vertex() {}
		Vertex(const glm::vec3& pos, glm::vec2 texCoord, const glm::vec3& normal, uint32_t mat) : Position(pos), Normal(normal), materialID(mat), TexCoords(texCoord) { }		
	};
}