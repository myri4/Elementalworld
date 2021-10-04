#pragma once
#include "Item.hpp"
#include <GUI/Renderer2D.hpp>
#include <GUI/Button.hpp>
#include <GUI/AssetManager.hpp>
#include <Utils/Keyboard.hpp>

namespace wc {

	template<size_t inventorySize>
	class InventoryContainer {
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

	template<size_t inventorySize, uint32_t sizeX, uint32_t sizeY, uint32_t hotbarSize>
	class Inventory : public InventoryContainer<inventorySize> {
	public:
		Button buttons[inventorySize];
		std::pair<ItemSlot, DragButton> buttonSlot;

		void Create() {
			for (uint16_t i = 0; i < inventorySize; i++) 
				buttons[i].size = { hotbarSize,hotbarSize };
		}

		void Update(const glm::vec2& windsize, const glm::vec2& windpos, const float& deltaTime, const Font& font) {
			float oneSixth = hotbarSize / 6.f;
			auto& data = this->data;

			for (uint16_t i = 0; i < inventorySize; i++)
			{
				uint16_t x = i % sizeX;
				uint16_t y = i / sizeX;
				buttons[i].position = glm::vec2((windsize.x - sizeX * hotbarSize) * 0.5f + (hotbarSize + 3.f) * x, (hotbarSize + 3.f) * y);
				Renderer2D::DrawQuad(buttons[i].position, buttons[i].size, assets.textures[1 + buttons[i].isMouseOver(windpos)]);

				if (buttons[i].isMouseOver(windpos) && (Mouse::isButtonPressed() == Mouse::MouseButton::LBUTTON && mouseUsed)) {
					if (buttonSlot.first.itemID == -1) { // Removes the item if the slot is empty 
						buttonSlot.first = data[i];
						data[i].itemID = -1;
						data[i].stack_size = 0;
						buttonSlot.second.position = buttons[i].position;
						buttonSlot.second.attach((glm::vec2)Mouse::GetMousePosToWindow(windpos));
					}
					else {
						if (buttonSlot.first.itemID != data[i].itemID) {
							auto item = data[i];
							data[i] = buttonSlot.first;
							buttonSlot.first = item;
						}
						else {
							data[i].stack_size += buttonSlot.first.stack_size;
							buttonSlot.first.stack_size = 0;
							buttonSlot.first.itemID = -1;
						}
					}
				}

				if (buttons[i].isMouseOver(windpos) && (Mouse::isButtonPressed() == Mouse::MouseButton::RBUTTON && mouseUsed) && buttonSlot.first.itemID != -1) {
					if (data[i].itemID == -1) data[i].itemID = buttonSlot.first.itemID;
					if (buttonSlot.first.stack_size > 0) {

						data[i].stack_size++;
						buttonSlot.first.stack_size--;
					}
					if (buttonSlot.first.stack_size == 0)
						buttonSlot.first.itemID = -1;
				}

				buttonSlot.second.updatePosition((glm::vec2)Mouse::GetMousePosToWindow(windpos));
				uint8_t& amount = data[i].stack_size;
				if (amount > 1) {
					Renderer2D::DrawQuad(buttons[i].position + 4.f, { hotbarSize - oneSixth,hotbarSize - oneSixth }, items[data[i].itemID].texture);
					Renderer2D::DrawText(std::to_string(amount), font, glm::vec2(buttons[i].position.x + 4.f, buttons[i].position.y + 44.f), 0.4f, glm::vec4(1.f));
				}
				else if (amount > 0) Renderer2D::DrawQuad(buttons[i].position + 4.f, { hotbarSize - oneSixth,hotbarSize - oneSixth }, items[data[i].itemID].texture);

				uint8_t& amount2 = buttonSlot.first.stack_size;
				if (amount2 > 0)
					Renderer2D::DrawQuad(buttonSlot.second.position + 4.f, { hotbarSize - oneSixth,hotbarSize - oneSixth }, items[buttonSlot.first.itemID].texture);
				if (amount2 > 1)
					Renderer2D::DrawText(std::to_string(amount2), font, glm::vec2(buttonSlot.second.position.x + 4.f, buttonSlot.second.position.y + 44.f), 0.4f, glm::vec4(1.f));
			}
		}

		void OnInput() {
			Mouse::ShowMouse(true);
			if (Keyboard::isKeyPressed(Keyboard::Key::E) && Action == GLFW_PRESS && keyPressed)
				mode = MenuMode::GAME;
		}
	};
}