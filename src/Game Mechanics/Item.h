#pragma once
#include "../world/Block.h"
#include <wc/Utils/List.h>

namespace wc {

	using ItemID = uint8_t;

	enum class ItemType : uint32_t {
		Block,
		Tool,
		Storage, // etc...
	};

	struct Item {
		uint32_t textureID = 0;
		uint32_t modelID = 0;
		union {
			uint32_t maxStackSize = 100;
			uint32_t maxDurability;
		};
		uint8_t block = 0; // @NOTE: maybe we should combine this with modelID
		ItemType type = ItemType::Block;
		uint8_t flags = 0;
		std::string name = "none";
		std::string displayName = "None";
	};

	List<Item, 256> itemData;

	struct ItemSlot {
		ItemID itemID = 0;
		union {
			uint32_t amount = 0;
			uint32_t durability;
		};
	};

	class Inventory {
	public:
		ItemSlot data[4 * 9]; // @TODO: probably should be with dynamic size

		void SetItem(const ItemSlot& itemSlot, const uint32_t& id) {
			data[id] = itemSlot;
		}

		bool PushItem(const ItemSlot& itemSlot) {
			for (int i = 0; i < std::size(data); i++) {
				if (itemSlot.itemID == data[i].itemID && data[i].amount + itemSlot.amount <= itemData[data[i].itemID].maxStackSize)
				{
					data[i].amount += itemSlot.amount;
					return true;
				}
			}

			return AddItem(itemSlot);
		}

		bool AddItem(const ItemSlot& itemSlot) {

			for (int i = 0; i < std::size(data); i++) {
				if (data[i].itemID == 0 || data[i].amount == 0)
				{
					SetItem(itemSlot, i);
					return true;
				}
			}

			return false;
		}

		bool RemoveItem(const uint32_t& id, const uint8_t& amount = 1) {
			if (int(data[id].amount - amount) > -1) {
				data[id].amount -= amount;
				return true;
			}

			return false;
		}

		void Sort() { // @TODO: here we will implement inventory sorting

		}
	};
}