#pragma once
#include <gl/Texture.hpp>
#include "../world/Block.hpp"
#include <Utils/List.hpp>

namespace wc {

	typedef uint8_t ItemID;

	struct Item {
		gl::Texture texture;
		uint8_t maxStackSize = 100;
		uint8_t block = 0; // @TODO: BlockID
	};

	List<Item, 256> itemData;

	struct ItemSlot {
		ItemID itemID = 0;
		uint8_t stack_size = 0;
	};
}