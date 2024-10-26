#pragma once
#include "Block.h"
#include <wc/Utils/List.h>

namespace wc {

	using ItemID = uint8_t;

	enum class ItemType : uint32_t {
		Block,
		Tool,
		Storage, // etc.
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
		ItemSlot m_Data[4 * 9]; // @TODO: probably should be with dynamic size
	public:

		ItemSlot& operator[](const size_t& index) { return m_Data[index]; }
		const ItemSlot& operator[](const size_t& index) const { return m_Data[index]; }
		constexpr size_t size() const { return std::size(m_Data); }

		bool PushItem(ItemSlot itemSlot) {
			for (int i = 0; i < std::size(m_Data); i++) {
				if (itemSlot.itemID == m_Data[i].itemID && m_Data[i].amount + itemSlot.amount <= itemData[m_Data[i].itemID].maxStackSize)
				{
					m_Data[i].amount += itemSlot.amount;
					return true;
				}
			}

			return AddItem(itemSlot);
		}

		bool AddItem(const ItemSlot& itemSlot) {

			for (int i = 0; i < std::size(m_Data); i++) {
				if (m_Data[i].itemID == 0 || m_Data[i].amount == 0)
				{
					m_Data[i] = itemSlot;
					return true;
				}
			}

			return false;
		}

		bool RemoveItem(uint32_t id, uint8_t amount = 1) {
			if (int(m_Data[id].amount - amount) > -1) {
				m_Data[id].amount -= amount;
				return true;
			}

			return false;
		}

		void Sort() { // @TODO: here we will implement inventory sorting

		}
	};
}