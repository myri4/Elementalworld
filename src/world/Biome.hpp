#ifndef BIOME_HPP
#define BIOME_HPP

#include "Block.hpp"
#include <Utils/CustomDefs.hpp>
#include <sol/sol.hpp>
#include <Maths/Noise.hpp>

namespace wc {
	
	class Biome {
	public:
		Biome(){}
		~Biome(){}
		void Create(const char* file) {
			sol::state luaState;

			luaState.open_libraries(sol::lib::base);
			//luaState.script_file(file);
		}
		void IsBiome(const Noise& biomeNoise, const int& y) {

		}
	private:
	};

}

#endif