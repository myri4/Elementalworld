#pragma once
//#include <gl/Texture.h>
#include "../world/Block.h"
#include <wc/Utils/List.h>

namespace wc {

	typedef uint8_t ItemID;

	struct Item {
		//gl::Texture texture;
		uint8_t maxStackSize = 100;
		uint8_t block = 0; // @TODO: BlockID
	};

	List<Item, 256> itemData;

	struct ItemSlot {
		ItemID itemID = 0;
		uint8_t stack_size = 0;
	};
}