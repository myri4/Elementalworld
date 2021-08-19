#ifndef INVENTORY_HPP
#define INVENTORY_HPP

#include "Item.hpp"

namespace wc {

	template<size_t inventorySize>
	class Inventory {
	public:
		ItemSlot data[inventorySize];

		bool AddItem(const uint8_t& itemID, const uint8_t& slot, const uint8_t& amount = 1) {
			auto findSlot = [&]() {
				for (uint8_t i = 0; i < inventorySize; i++) {
					if (data[i].itemID == -1) { // if empty
						data[i].itemID = itemID;
						if (data[i].stack_size + amount <= items[itemID].maxStackSize) {
							data[i].stack_size += amount;
							return true;
						}
					}
					else if (data[i].itemID == itemID) { // if the items are the same
						if (data[i].stack_size + amount <= items[itemID].maxStackSize) {
							data[i].stack_size += amount;
							return true;
						}
					}
				}
				return false;
			};

			if (data[slot].itemID == -1) { // if empty
				data[slot].itemID = itemID;
				if (data[slot].stack_size + amount <= items[itemID].maxStackSize) {
					data[slot].stack_size += amount;
					return true;
				}
				else
					return findSlot();
			}
			else if (data[slot].itemID == itemID) { // if the items are the same
				if (data[slot].stack_size + amount <= items[itemID].maxStackSize) {
					data[slot].stack_size += amount;
					return true;
				}
				else
					return findSlot();
			}
			else  // if they are not the same
				return findSlot();

			return false;
		}

		bool RemoveItem(const uint32_t& slot, const uint32_t& amount = 1) {
			if (data[slot].itemID == -1) return false;
			if ((int)(data[slot].stack_size - amount) >= 0) {
				data[slot].stack_size -= amount;

				if (data[slot].stack_size == 0) data[slot].itemID = -1;
				return true;
			}
			return false;
		}
	};
}

#endif