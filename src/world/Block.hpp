#ifndef BLOCK_HPP
#define BLOCK_HPP

#include <glm/glm.hpp>

namespace wc{

enum ConnectionType : uint8_t { CONNECT_DEFAULT, FLUID_CONNECT, NO_CONNECT, X_CONNECT, CANT_CONNECT};
enum class BlockTexture : uint8_t { TOP, BOTTOM, LEFT, RIGHT, FRONT, BACK };

const float blockSize = 1.f;

struct Face {
	glm::vec3 corner1;
	glm::vec3 corner2;
	glm::vec3 corner3;
	glm::vec3 corner4;
	BlockTexture texID;
	glm::vec3 normal;

	void CalculateNormal() {
		normal = glm::cross(corner3 - corner1, corner2 - corner1);		
	}
};

typedef int8_t BlockID;

Face BACK_FACE = {
	glm::vec3( 0.f,  blockSize, 0.f), // top-left
	glm::vec3( 0.f, 0.f, 0.f), // Bottom-left  
	glm::vec3( blockSize, 0.f, 0.f), // bottom-right 
	glm::vec3( blockSize,  blockSize, 0.f), // top-right
	BlockTexture::BACK
};

Face FRONT_FACE = {
	glm::vec3( blockSize,  blockSize,  blockSize), // top-right
	glm::vec3( blockSize, 0.f,  blockSize), // bottom-right        
	glm::vec3( 0.f, 0.f,  blockSize), // bottom-left
	glm::vec3( 0.f,  blockSize,  blockSize), // top-left   
	BlockTexture::FRONT
};

Face LEFT_FACE = {
	glm::vec3(0.f,  blockSize,  blockSize),  // top-right
	glm::vec3(0.f, 0.f,  blockSize),  // bottom-right
	glm::vec3(0.f, 0.f, 0.f),  // bottom-left 
	glm::vec3(0.f,  blockSize, 0.f),  // top-left
	BlockTexture::LEFT
};

Face RIGHT_FACE = {
	 glm::vec3(blockSize,  blockSize, 0.f),  // top-right      
	 glm::vec3(blockSize, 0.f, 0.f),  // bottom-right          
	 glm::vec3(blockSize, 0.f,  blockSize),  // bottom-left
	 glm::vec3(blockSize,  blockSize,  blockSize),  // top-left
	 BlockTexture::RIGHT
};

Face BOTTOM_FACE = {
	glm::vec3(0.f, 0.f, 0.f),  // top-right 
	glm::vec3(0.f, 0.f,  blockSize),  // bottom-right
	glm::vec3( blockSize, 0.f,  blockSize),  // bottom-left
	glm::vec3( blockSize, 0.f, 0.f),  // top-left  
	BlockTexture::BOTTOM
};

Face TOP_FACE = {
	glm::vec3( blockSize,  blockSize, 0.f), // top-right
	glm::vec3( blockSize,  blockSize,  blockSize), // bottom-right                 
	glm::vec3(0.f,  blockSize,  blockSize), // bottom-left  
	glm::vec3(0.f,  blockSize, 0.f), // top-left 
	BlockTexture::TOP
};

Face X_FACE1 = {
	glm::vec3( blockSize,  blockSize, 0.f),  // top-right      
	glm::vec3( blockSize, 0.f, 0.f),  // bottom-right          
	glm::vec3(0.f, 0.f,  blockSize),  // bottom-left
	glm::vec3(0.f,  blockSize,  blockSize),  // top-left
	BlockTexture::TOP
};

Face X_FACE2 = {
	glm::vec3( blockSize,  blockSize,  blockSize),  // top-right
	glm::vec3( blockSize, 0.f,  blockSize),  // bottom-right
	glm::vec3(0.f, 0.f, 0.f),  // bottom-left 
	glm::vec3(0.f,  blockSize, 0.f),  // top-left
	BlockTexture::TOP
};

struct Block{
	BlockID id = 0;

	bool isCollidable = true;
	bool emitLight = false;

	uint32_t texture[6] = {0};
	uint32_t normalTexture[6] = { 0 };
	uint8_t blockConnectionType = ConnectionType::CONNECT_DEFAULT;

	Block() = default;
};
}
#endif 