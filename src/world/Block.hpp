#pragma once

#include <lua/lua.hpp>
#include <sol/sol.hpp>
#include <gl/Material.hpp>

namespace wc{
enum class ConnectionType { CONNECT_DEFAULT, FLUID_CONNECT, NO_CONNECT};
enum class BlockTexture { TOP, BOTTOM, LEFT, RIGHT, FRONT, BACK };

static const float blockSize = 0.5f;

using Face = std::array < glm::vec3, 4 >;

Face BACK_FACE = {
	glm::vec3(-blockSize,  blockSize, -blockSize), // top-left
	glm::vec3(-blockSize, -blockSize, -blockSize), // Bottom-left  
	glm::vec3( blockSize, -blockSize, -blockSize), // bottom-right 
	glm::vec3( blockSize,  blockSize, -blockSize), // top-right
};

Face FRONT_FACE = {
	glm::vec3( blockSize,  blockSize,  blockSize), // top-right
	glm::vec3( blockSize, -blockSize,  blockSize), // bottom-right        
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
	glm::vec3( blockSize, -blockSize,  blockSize),  // bottom-left
	glm::vec3( blockSize, -blockSize, -blockSize)   // top-left  
};

Face TOP_FACE = {
	glm::vec3( blockSize,  blockSize, -blockSize), // top-right
	glm::vec3( blockSize,  blockSize,  blockSize), // bottom-right                 
	glm::vec3(-blockSize,  blockSize,  blockSize), // bottom-left  
	glm::vec3(-blockSize,  blockSize, -blockSize)  // top-left 
};

class Block{
public:
	uint32_t id = 0;
	bool isCollidable = true;
	glm::vec2 TexCoords[6];
	gl::Material material;
	ConnectionType blockConnectionType = ConnectionType::CONNECT_DEFAULT;

	Block(const char* file) {Create(file);}
	Block() {}
	void Create(const char* file) {
		std::string conType;
		sol::state blockState;
		blockState.script_file(file);
		if (blockState["id"].valid()) id = blockState["id"];
		if (blockState["isCollidable"].valid()) isCollidable = blockState["isCollidable"];
		if (blockState["ConnectionType"].valid()) conType = blockState["ConnectionType"];

		if (conType == "CONNECT_DEFAULT") blockConnectionType = ConnectionType::CONNECT_DEFAULT;
		if (conType == "FLUID_CONNECT") blockConnectionType = ConnectionType::FLUID_CONNECT;
		if (conType == "NO_CONNECT") blockConnectionType = ConnectionType::NO_CONNECT;
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