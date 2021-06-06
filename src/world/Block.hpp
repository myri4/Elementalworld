#ifndef BLOCK_HPP
#define BLOCK_HPP

#include <glm/glm.hpp>

namespace wc{

enum class ConnectionType { CONNECT_DEFAULT, FLUID_CONNECT, NO_CONNECT, X_CONNECT, CANT_CONNECT};
enum class BlockTexture { TOP, BOTTOM, LEFT, RIGHT, FRONT, BACK };

static const float blockSize = 0.5f;

struct Face {
	glm::vec3 corner1;
	glm::vec3 corner2;
	glm::vec3 corner3;
	glm::vec3 corner4;
	BlockTexture texID;
};

typedef int8_t BlockID;

Face BACK_FACE = {
	glm::vec3(-blockSize,  blockSize, -blockSize), // top-left
	glm::vec3(-blockSize, -blockSize, -blockSize), // Bottom-left  
	glm::vec3( blockSize, -blockSize, -blockSize), // bottom-right 
	glm::vec3( blockSize,  blockSize, -blockSize), // top-right
	BlockTexture::BACK
};

Face FRONT_FACE = {
	glm::vec3( blockSize,  blockSize,  blockSize), // top-right
	glm::vec3( blockSize, -blockSize,  blockSize), // bottom-right        
	glm::vec3(-blockSize, -blockSize,  blockSize), // bottom-left
	glm::vec3(-blockSize,  blockSize,  blockSize), // top-left   
	BlockTexture::FRONT
};

Face LEFT_FACE = {
	glm::vec3(-blockSize,  blockSize,  blockSize),  // top-right
	glm::vec3(-blockSize, -blockSize,  blockSize),  // bottom-right
	glm::vec3(-blockSize, -blockSize, -blockSize),  // bottom-left 
	glm::vec3(-blockSize,  blockSize, -blockSize),  // top-left
	BlockTexture::LEFT
};

Face RIGHT_FACE = {
	 glm::vec3(blockSize,  blockSize, -blockSize),  // top-right      
	 glm::vec3(blockSize, -blockSize, -blockSize),  // bottom-right          
	 glm::vec3(blockSize, -blockSize,  blockSize),  // bottom-left
	 glm::vec3(blockSize,  blockSize,  blockSize),  // top-left
	 BlockTexture::RIGHT
};

Face BOTTOM_FACE = {
	glm::vec3(-blockSize, -blockSize, -blockSize),  // top-right 
	glm::vec3(-blockSize, -blockSize,  blockSize),  // bottom-right
	glm::vec3( blockSize, -blockSize,  blockSize),  // bottom-left
	glm::vec3( blockSize, -blockSize, -blockSize),  // top-left  
	BlockTexture::BOTTOM
};

Face TOP_FACE = {
	glm::vec3( blockSize,  blockSize, -blockSize), // top-right
	glm::vec3( blockSize,  blockSize,  blockSize), // bottom-right                 
	glm::vec3(-blockSize,  blockSize,  blockSize), // bottom-left  
	glm::vec3(-blockSize,  blockSize, -blockSize), // top-left 
	BlockTexture::TOP
};

Face X_FACE1 = {
	glm::vec3( blockSize,  blockSize, -blockSize),  // top-right      
	glm::vec3( blockSize, -blockSize, -blockSize),  // bottom-right          
	glm::vec3(-blockSize, -blockSize,  blockSize),  // bottom-left
	glm::vec3(-blockSize,  blockSize,  blockSize),  // top-left
	BlockTexture::TOP
};

Face X_FACE2 = {
	glm::vec3( blockSize,  blockSize,  blockSize),  // top-right
	glm::vec3( blockSize, -blockSize,  blockSize),  // bottom-right
	glm::vec3(-blockSize, -blockSize, -blockSize),  // bottom-left 
	glm::vec3(-blockSize,  blockSize, -blockSize),  // top-left
	BlockTexture::TOP
};

struct Block{
	BlockID id = 0;

	bool isCollidable = true;
	//bool emitLight = false;

	uint32_t texture[6] = {0};
	ConnectionType blockConnectionType = ConnectionType::CONNECT_DEFAULT;

	Block() = default;
};
}
#endif 