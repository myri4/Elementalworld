#ifndef ITEM_HPP
#define ITEM_HPP

#include <gl/Texture.hpp>
#include "../world/Block.hpp"

namespace wc {

	struct Item {
		gl::Texture texture;
		uint8_t id = 0;
		uint8_t maxStackSize = 100;
		BlockID block = 0u;
	} items[256];

	struct ItemSlot {
		int8_t itemID = -1;
		uint8_t stack_size = 0;
	};
}

#endif