#pragma once
#include "Item.h"
#include <GUI/AssetManager.h>

namespace wc {

	template<size_t inventorySize>
	struct InventoryContainer {
		ItemSlot data[inventorySize];

		bool AddItem(const ItemID& itemID, const uint8_t& slot, const uint8_t& amount = 1) {
			auto findSlot = [&]() {
				for (uint8_t i = 0; i < inventorySize; i++) {
					if (data[i].itemID == 0) { // if empty
						data[i].itemID = itemID;
						if (data[i].stack_size + amount <= itemData[itemID].maxStackSize) {
							data[i].stack_size += amount;
							return true;
						}
					}
					else if (data[i].itemID == itemID) { // if the items are the same
						if (data[i].stack_size + amount <= itemData[itemID].maxStackSize) {
							data[i].stack_size += amount;
							return true;
						}
					}
				}
				return false;
			};

			if (data[slot].itemID == 0) { // if empty
				data[slot].itemID = itemID;
				if (data[slot].stack_size + amount <= itemData[itemID].maxStackSize) {
					data[slot].stack_size += amount;
					return true;
				}
				else
					return findSlot();
			}
			else if (data[slot].itemID == itemID) { // if the items are the same
				if (data[slot].stack_size + amount <= itemData[itemID].maxStackSize) {
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

		bool RemoveItem(const uint32_t& slot, const uint8_t& amount = 1) {
			int diff = data[slot].stack_size - amount;
			if (diff >= 0) {
				data[slot].stack_size = diff;

				if (data[slot].stack_size == 0) data[slot].itemID = 0;
				return true;
			}
			return false;
		}
	};

	template<uint32_t size>
	class Recipe {
	public:
		ItemID data[size * size] = { 0 };
		ItemID result = 0;
		uint8_t amount = 0;	
	};
	Recipe<2> recipes[2];
	
	/*template<uint32_t size>
	class Crafter{
	public:
		void UpdateLogic(ItemSlot* data) {			
			int32_t recipeID = -1;
			for (uint32_t j = 0; j < ARRAYSIZE(recipes); j++) {
				recipeID = j;
				for (uint32_t i = 0; i < size * size; i++) {
					if (data[i].itemID != recipes[j].data[i]) {
						recipeID = -1;
						break;
					}
				}

				if (recipeID > -1) break;
			}

			if (recipeID > -1) {
				data[size * size].itemID = recipes[recipeID].result;
				data[size * size].stack_size = recipes[recipeID].amount;
			}
			else {
				data[size * size].itemID = 0;
				data[size * size].stack_size = 0;
			}
		}
	};*/
}