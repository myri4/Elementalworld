#pragma once
#include "Item.hpp"
#include <GUI/Renderer2D.hpp>
#include <GUI/Button.hpp>
#include <GUI/AssetManager.hpp>
#include "../Menus/Menus.hpp"

namespace wc {

	std::pair<ItemSlot, DragButton> buttonSlot;
	const uint32_t hotbarSize = 48;
	template<size_t inventorySize>
	class InventoryContainer {
	public:
		ItemSlot data[inventorySize];

		bool AddItem(const uint8_t& itemID, const uint8_t& slot, const uint8_t& amount = 1) {
			auto findSlot = [&]() {
				for (uint8_t i = 0; i < inventorySize; i++) {
					if (data[i].itemID == 0) { // if empty
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

			if (data[slot].itemID == 0) { // if empty
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
			if (data[slot].itemID == 0) return false;
			if ((int)(data[slot].stack_size - amount) >= 0) {
				data[slot].stack_size -= amount;

				if (data[slot].stack_size == 0) data[slot].itemID = 0;
				return true;
			}
			return false;
		}
	};

	template<uint32_t sizeX, uint32_t sizeY, uint32_t size>
	class InventoryMenu : public InventoryContainer<size> {
	public:
		Button buttons[size];

		void Create() {
			for (uint16_t i = 0; i < size; i++)
				buttons[i].size = { hotbarSize,hotbarSize };
		}

		void Update(const glm::vec2& windsize, const glm::vec2& windpos, const float& deltaTime, const Font& font) {
			UpdateLogic(windsize, windpos, deltaTime, font, this->data);
		}

		virtual void UpdateLogic(const glm::vec2& windsize, const glm::vec2& windpos, const float& deltaTime, const Font& font, ItemSlot* data) { 
			Display(windsize, windpos, deltaTime, font);
		}

		virtual void UpdatePosition(const glm::vec2& windsize, const glm::vec2& windpos, const uint32_t& x, const uint32_t& y, const uint32_t& i, ItemSlot* data) {
			buttons[i].position = glm::vec2((windsize.x - sizeX * hotbarSize) * 0.5f + (hotbarSize + 3.f) * x, (hotbarSize + 3.f) * y);
		}

		virtual void UpdateButtonInput(const uint32_t& i, ItemSlot* data) {
			if (buttons[i].isMouseOver() && (Mouse::getMouse(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)) {
				if (buttonSlot.first.itemID == 0) { // Removes the item if the slot is empty 
					buttonSlot.first = data[i];
					data[i].itemID = 0;
					data[i].stack_size = 0;
					buttonSlot.second.position = buttons[i].position;
					buttonSlot.second.attach(Mouse::GetMousePosToWindow());
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
						buttonSlot.first.itemID = 0;
					}
				}
			}

			if (buttons[i].isMouseOver() && Mouse::getMouse(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && buttonSlot.first.itemID != -1) {
				if (data[i].itemID == 0) data[i].itemID = buttonSlot.first.itemID;
				if (buttonSlot.first.stack_size > 0) {

					data[i].stack_size++;
					buttonSlot.first.stack_size--;
				}
				if (buttonSlot.first.stack_size == 0)
					buttonSlot.first.itemID = 0;
			}
		}

		void Display(const glm::vec2& windsize, const glm::vec2& windpos, const float& deltaTime, const Font& font) {
			float oneSixth = hotbarSize / 6.f;
			auto& data = this->data;

			for (uint16_t i = 0; i < size; i++)
			{
				uint16_t x = i % sizeX;
				uint16_t y = i / sizeX;
				UpdatePosition(windsize, windpos, x, y, i, data);
				Renderer2D::DrawQuad(buttons[i].position, buttons[i].size, assets.textures[1 + buttons[i].isMouseOver()]);

				UpdateButtonInput(i, data);

				buttonSlot.second.updatePosition(Mouse::GetMousePosToWindow());
				uint8_t& amount = data[i].stack_size;
				if (amount > 1) {
					Renderer2D::DrawQuad(buttons[i].position + 4, { hotbarSize - oneSixth,hotbarSize - oneSixth }, items[data[i].itemID].texture);
					Renderer2D::DrawText(std::to_string(amount), font, glm::vec2(buttons[i].position.x + 4.f, buttons[i].position.y + 44.f), 0.4f, glm::vec4(1.f));
				}
				else if (amount > 0) Renderer2D::DrawQuad(buttons[i].position + 4, { hotbarSize - oneSixth,hotbarSize - oneSixth }, items[data[i].itemID].texture);

				uint8_t& amount2 = buttonSlot.first.stack_size;
				if (amount2 > 0)
					Renderer2D::DrawQuad(buttonSlot.second.position + 4, { hotbarSize - oneSixth,hotbarSize - oneSixth }, items[buttonSlot.first.itemID].texture);
				if (amount2 > 1)
					Renderer2D::DrawText(std::to_string(amount2), font, glm::vec2(buttonSlot.second.position.x + 4.f, buttonSlot.second.position.y + 44.f), 0.4f, glm::vec4(1.f));
			}
		}

		void OnInput() {
			Mouse::ShowMouse(true);
			if (Keyboard::getKey(Keyboard::Key::E) == GLFW_PRESS)
				mode = MenuMode::GAME;
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
	
	template<uint32_t size>
	class Crafter : public InventoryMenu<size, size, size * size + 1>{
	public:
		void UpdateLogic(const glm::vec2& windsize, const glm::vec2& windpos, const float& deltaTime, const Font& font, ItemSlot* data) override {			
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

			this->Display(windsize, windpos, deltaTime, font); 
		}
		void UpdatePosition(const glm::vec2& windsize, const glm::vec2& windpos, const uint32_t& x, const uint32_t& y, const uint32_t& i, ItemSlot* data) override {
			if (i != size * size) this->buttons[i].position = glm::vec2((windsize.x - size * hotbarSize) * 0.5f + (hotbarSize + 3.f) * x, (hotbarSize + 3.f) * y + 400.f);
			else 
				this->buttons[i].position = glm::vec2((windsize.x - size * hotbarSize) * 0.5f + (hotbarSize + 3.f) * x + 20.f, (hotbarSize + 3.f) * y + 430.f);
		}

		void UpdateButtonInput(const uint32_t& i, ItemSlot* data) override {
			auto& buttons = this->buttons;
			if (buttons[i].isMouseOver() && Mouse::getMouse(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
				if (buttonSlot.first.itemID == 0) { // Removes the item if the slot is empty 
					if (i == size * size && data[i].itemID != -1) {
						for (uint32_t j = 0; j < size * size; j++) {
							if (data[j].stack_size > 0) data[j].stack_size--;
							if (data[j].stack_size == 0) data[j].itemID = -1;
						}
					}
					buttonSlot.second.attach(Mouse::GetMousePosToWindow());
					buttonSlot.first = data[i];
					data[i].itemID = 0;
					data[i].stack_size = 0;
					buttonSlot.second.position = buttons[i].position;
				}
			}

			if (buttons[i].isMouseOver() && Mouse::getMouse(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && buttonSlot.first.itemID != -1 && i != size * size) {
				if (data[i].itemID == 0) data[i].itemID = buttonSlot.first.itemID;
				if (buttonSlot.first.stack_size > 0) {

					data[i].stack_size++;
					buttonSlot.first.stack_size--;
				}
				if (buttonSlot.first.stack_size == 0)
					buttonSlot.first.itemID = 0;
			}
		}
	};
}