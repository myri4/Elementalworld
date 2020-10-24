#pragma once

#include <wclibs/pch.hpp>

#include "Block.hpp"

namespace wc {

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

class Chunk {
public: // Variables
	glm::vec3 chunkPosition = { 0,0,0 };
	int8_t chunkData[chunkSize][chunkSize][chunkSize];
	bool used = false;

	gl::VertexBuffer chunkMeshBuffer;
	gl::IndexBuffer chunkIndexBuffer;
	gl::Vertex chunkMesh[MaxVertexCount];

	uint32_t IndexCount = 0;
	uint32_t offset = 0;

public: // Functions 
	Chunk() {}
	Chunk(const glm::vec3& pos) { Create(pos); }
	~Chunk() {}
	void Create(const glm::vec3& pos) {
		chunkPosition = pos;
		//Configuring the vertex array
		chunkMeshBuffer.Create(nullptr, sizeof(chunkMesh), GL_DYNAMIC_DRAW);
		gl::VertexAttribPointer(0, 3, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, Position));  // position attribute
		gl::VertexAttribPointer(1, 2, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, TexCoords)); // texture coord attribute
		gl::VertexAttribPointer(2, 3, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, Normal));    // Normal attribute

		//Generating index buffer
		uint32_t indices[MaxIndexCount];
		uint32_t iOffset = 0;
		for (uint32_t i = 0; i < MaxIndexCount; i += 6) {
			indices[i + 0] = 0 + iOffset;
			indices[i + 1] = 1 + iOffset;
			indices[i + 2] = 2 + iOffset;

			indices[i + 3] = 2 + iOffset;
			indices[i + 4] = 3 + iOffset;
			indices[i + 5] = 0 + iOffset;

			iOffset += 4;
		}
		chunkIndexBuffer.Create(indices, sizeof(indices));

		WC_INFO("Created chunk at X:{0} Y:{1} Z:{2}", pos.x, pos.y, pos.z);
	}
};
}