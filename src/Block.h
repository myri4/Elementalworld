#pragma once

#include <wclibs/pch.hpp>
   
namespace wc{
enum BlockType { Fluid, Solid, Air, Leave };
enum BlockTexture{TOP_TEXTURE, LEFT_TEXTURE, RIGHT_TEXTURE, FRONT_TEXTURE, BACK_TEXTURE, BOTTOM_TEXTURE};
class Block{
public:
	uint32_t id;
	bool isCollidable;
	gl::Texture blockTexture[6];

	Block(const char* file) {Create(file);}
	Block() {}
	void Create(const char* file) {
		wc::Lua blockState(file);
		id = blockState.GetNumber("id");
		isCollidable = blockState.GetBool("isCollidable");
		blockTexture[TOP_TEXTURE].load(blockState.GetString("Top"));
		blockTexture[LEFT_TEXTURE].load(blockState.GetString("Left"));
		blockTexture[RIGHT_TEXTURE].load(blockState.GetString("Right"));
		blockTexture[FRONT_TEXTURE].load(blockState.GetString("Front"));
		blockTexture[BACK_TEXTURE].load(blockState.GetString("Back"));
		blockTexture[BOTTOM_TEXTURE].load(blockState.GetString("Bottom"));
	}
private:

};
}