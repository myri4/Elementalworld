#ifndef ITEM_HPP
#define ITEM_HPP

#include "../world/World.hpp"

namespace wc {

	class Item {
	public:
		Item() {}
		Item(const char* _name, const gl::Texture& tex) : name(_name), texture(tex) {}
		virtual bool onInteract() { return false; }
		virtual bool onUse() { return false; }
		virtual uint32_t GetDurabiliy() { return 0; }

		const char* name = nullptr;
		gl::Texture texture;
		uint32_t maxStackSize = 1;
		uint32_t stackSize = 1;
	};
}

#endif