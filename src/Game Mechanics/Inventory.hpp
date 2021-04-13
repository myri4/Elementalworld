#ifndef INVENTORY_HPP
#define INVENTORY_HPP

#include "Item.hpp"
#include <glm/glm.hpp>

class BlockItem : public wc::Item {
	bool onUse() override {
		return true;
	}
}testItem;

namespace wc {

template<uint8_t sizeX, uint8_t sizeY>
class Inventory {
public:
	Item data[sizeX][sizeY];

	bool RemoveItem() { data[1][1] = testItem; return true; }
	bool AddItem() { return false; }
	bool HasItem() { return false; }
};
}

#endif