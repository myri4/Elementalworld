#pragma once
#include <gl/Texture.hpp>
#include "../world/Block.hpp"

namespace wc {

	typedef uint8_t ItemID;
	uint32_t numItems = 0;

	struct Item {
		gl::Texture texture;
		uint8_t maxStackSize = 100;
		uint8_t block = 0; // @TODO: BlockID
	} items[256];

	struct ItemSlot {
		ItemID itemID = 0;
		uint8_t stack_size = 0;
	};
}