#pragma once

#include <Utilitiess/Lua.hpp>
#include <gl/Material.hpp>

namespace wc{
enum class BlockType { Fluid, Solid, Air, Leave };
enum class BlockTexture { TOP, BOTTOM, LEFT, RIGHT, FRONT, BACK };

static const float blockSize = 0.5f;

using Face = std::array < glm::vec3, 4 >;

Face BACK_FACE = {
	glm::vec3(-blockSize,  blockSize, -blockSize), // top-left
	glm::vec3(-blockSize, -blockSize, -blockSize), // Bottom-left  
	glm::vec3(blockSize, -blockSize, -blockSize), // bottom-right 
	glm::vec3(blockSize,  blockSize, -blockSize), // top-right
};

Face FRONT_FACE = {
	glm::vec3(blockSize,  blockSize,  blockSize), // top-right
	glm::vec3(blockSize, -blockSize,  blockSize), // bottom-right        
	glm::vec3(-blockSize, -blockSize,  blockSize), // bottom-left
	glm::vec3(-blockSize,  blockSize,  blockSize)  // top-left   
};

Face LEFT_FACE = {
	glm::vec3(-blockSize,  blockSize,  blockSize),  // top-right
	glm::vec3(-blockSize, -blockSize,  blockSize),  // bottom-right
	glm::vec3(-blockSize, -blockSize, -blockSize),  // bottom-left 
	glm::vec3(-blockSize,  blockSize, -blockSize)   // top-left   
};

Face RIGHT_FACE = {
	 glm::vec3(blockSize,  blockSize, -blockSize),  // top-right      
	 glm::vec3(blockSize, -blockSize, -blockSize),  // bottom-right          
	 glm::vec3(blockSize, -blockSize,  blockSize),  // bottom-left
	 glm::vec3(blockSize,  blockSize,  blockSize)   // top-left
};

Face BOTTOM_FACE = {
	glm::vec3(-blockSize, -blockSize, -blockSize),  // top-right 
	glm::vec3(-blockSize, -blockSize,  blockSize),  // bottom-right
	glm::vec3(blockSize, -blockSize,  blockSize),  // bottom-left
	glm::vec3(blockSize, -blockSize, -blockSize)   // top-left  
};

Face TOP_FACE = {
	glm::vec3(blockSize,  blockSize, -blockSize), // top-right
	glm::vec3(blockSize,  blockSize,  blockSize), // bottom-right                 
	glm::vec3(-blockSize,  blockSize,  blockSize), // bottom-left  
	glm::vec3(-blockSize,  blockSize, -blockSize)  // top-left 
};

class Block{
public:
	uint32_t id = 0;
	bool isCollidable = true;
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
};
}