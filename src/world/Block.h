#pragma once
#include <glm/glm.hpp>
#include "Item.h"
#include "../Globals.h"

namespace wc {

	enum ConnectionType : uint8_t {
		CONNECT_DEFAULT, FLUID_CONNECT, NO_CONNECT,
		SLAB_DOWN, SLAB_UP, SLAB_LEFT, SLAB_RIGHT, SLAB_FRONT, SLAB_BACK,
		CANT_CONNECT, AIR, CUSTOM_MODEL, NON_EXISTENT
	};
	enum class BlockTexture : uint8_t { RIGHT, LEFT, TOP, BOTTOM, FRONT, BACK };

	const uint32_t WC_MODEL_BIT = 0x1;
	const uint32_t WC_CULL_BIT = 0x2;

	using BlockID = uint8_t;
	using MaterialID = uint8_t;
	using MeshID = uint32_t;

	struct Block {
		ConnectionType connectionType = ConnectionType::AIR;

		bool isCollidable = false;
		MeshID meshID = 0;
		uint8_t variations = 0;
		MaterialID materialIDs[6] = { 0 };
		glm::vec3 rotation = glm::vec3(0.f);

		uint32_t flags = 0;

		std::string name = "air";

		Block() {}
	};
}