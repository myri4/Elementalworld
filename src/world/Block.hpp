#pragma once

#include <Utilitiess/Lua.hpp>
#include <gl/Material.hpp>

namespace wc{
enum class BlockType { Fluid, Solid, Air, Leave };
enum class BlockTexture { TOP, LEFT, RIGHT, FRONT, BACK, BOTTOM };
static const float blockSize = 0.5f;
class Block{
public:
	uint32_t id = 0;
	bool isCollidable = true;
	//gl::Texture blockTexture[6];
	glm::vec2 TexCoords[6];
	gl::Material material;

	Block(const char* file) {Create(file);}
	Block() {}
	void Create(const char* file) {
		wc::Lua blockState(file);
		id = blockState.GetNumber("id");
		isCollidable = blockState.GetBool("isCollidable");
		//blockTexture[TOP_TEXTURE].load(blockState.GetString("Top"));
		//blockTexture[LEFT_TEXTURE].load(blockState.GetString("Left"));
		//blockTexture[RIGHT_TEXTURE].load(blockState.GetString("Right"));
		//blockTexture[FRONT_TEXTURE].load(blockState.GetString("Front"));
		//blockTexture[BACK_TEXTURE].load(blockState.GetString("Back"));
		//blockTexture[BOTTOM_TEXTURE].load(blockState.GetString("Bottom"));
	}
	void SendMaterialToShader(const gl::Shader &shader, const std::string& materialUnif) {material.Apply(shader, materialUnif);}
private:

};
}