#ifndef BLOCK_HPP
#define BLOCK_HPP

#include <glm/glm.hpp>
#include <array>

namespace wc{

enum class ConnectionType { CONNECT_DEFAULT, FLUID_CONNECT, NO_CONNECT, X_CONNECT};
enum class BlockTexture { TOP, BOTTOM, LEFT, RIGHT, FRONT, BACK };

static const float blockSize = 0.5f;

using Face = std::array < glm::vec3, 4 >;
typedef int8_t BlockID;

/*template<uint32_t Size>
	class Model {
	public:
		int Size() const { return Size; }

		gl::Vertex& operator[](size_t index) { if (!(index < Size))__debugbreak(); return data[index]; }
		const gl::Vertex& operator[](size_t index) const { if (!(index < Size))__debugbreak(); return data[index]; }

		gl::Vertex* data() { return data; }
		const gl::Vertex* data() const { return data; }
	private:
		gl::Vertex data[Size];
	};*/


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

Face X_FACE1 = {
	glm::vec3( blockSize,  blockSize, -blockSize),  // top-right      
	glm::vec3( blockSize, -blockSize, -blockSize),  // bottom-right          
	glm::vec3(-blockSize, -blockSize,  blockSize),  // bottom-left
	glm::vec3(-blockSize,  blockSize,  blockSize)   // top-left
};

Face X_FACE2 = {
	glm::vec3( blockSize,  blockSize,  blockSize),  // top-right
	glm::vec3( blockSize, -blockSize,  blockSize),  // bottom-right
	glm::vec3(-blockSize, -blockSize, -blockSize),  // bottom-left 
	glm::vec3(-blockSize,  blockSize, -blockSize)   // top-left   
};

Face X_FACE3 = {
	glm::vec3(-blockSize,  blockSize, -blockSize),  // top-right      
	glm::vec3(-blockSize, -blockSize, -blockSize),  // bottom-right          
	glm::vec3( blockSize, -blockSize,  blockSize),  // bottom-left
	glm::vec3( blockSize,  blockSize,  blockSize)   // top-left
};

Face X_FACE4 = {
	glm::vec3(-blockSize,  blockSize,  blockSize),  // top-right
	glm::vec3(-blockSize, -blockSize,  blockSize),  // bottom-right
	glm::vec3( blockSize, -blockSize, -blockSize),  // bottom-left 
	glm::vec3( blockSize,  blockSize, -blockSize)   // top-left   
};

const uint8_t isCollidableFlag = 1; // 0
const uint8_t emitLightFlag = 2;	// 1

class Block{
public:
	BlockID id = 0;
	uint8_t flags = isCollidableFlag;
	uint32_t texture[6] = {0};
	ConnectionType blockConnectionType = ConnectionType::CONNECT_DEFAULT;

	Block() {}
};
}
#endif 