#ifndef BIOME_HPP
#define BIOME_HPP

#include "Block.hpp"
#include <Utils/CustomDefs.hpp>
#include <sol/sol.hpp>

namespace wc {

	class Biome {
	public:
		BlockID topBlock;
		int16_t biomeValue;
		Biome(){}
		~Biome(){}
		void Create(const char* file) {
			sol::state luaState;
			luaState.script_file(file);

		}
	private:

	};

}

#endif