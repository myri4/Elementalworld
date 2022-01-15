#pragma once
#include <gl/Texture.hpp>
#include "../world/Block.hpp"

namespace wc {

	typedef uint8_t ItemID;

	struct Item {
		gl::Texture texture;
		uint8_t maxStackSize = 100;
		BlockID block = 0u;
	} items[256];

	struct ItemSlot {
		ItemID itemID = 0;
		uint8_t stack_size = 0;
	};
}