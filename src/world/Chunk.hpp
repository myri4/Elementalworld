#pragma once

#include <wclibs/pch.hpp>

#include "Block.hpp"

namespace wc {

Block grassBlock;

std::vector<Block> blockData;

static const float blockSize = 0.5f;

using Face = std::array < glm::vec3, 4 >;

gl::Texture blockAtlas;

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

		UpdateMesh();
	}
	void Update() {

	}
	void Draw(const gl::Shader& shader) {
		if (chunkMeshBuffer.GetVAO() == 0 || chunkIndexBuffer.GetEBO() == 0) return;
		// calculate the model matrix for each object and pass it to shader before drawing
		shader.use();
		shader.setMat4("model", glm::translate(glm::mat4(1.0f), chunkPosition * glm::vec3(chunkSize)));
		UpdateMesh();
	}
	void setBlock(const glm::vec3& pos, const int& block) { // TEMP:
		if (pos.x > chunkSize || pos.y > chunkSize || pos.z > chunkSize) return;
		int x = pos.x;
		int y = pos.y;
		int z = pos.z;
		chunkData[x][y][z] = block;
	}
	
	void UpdateMesh() {
		ResetMesh();
		BeginBatch();
		for (uint8_t y = 0; y < chunkSize; y++)
			for (uint8_t z = 0; z < chunkSize; z++)
				for (uint8_t x = 0; x < chunkSize; x++)
				{
					if (chunkData[x][y][z] > 0) {
					//Positive
						if (chunkData[x + 1][y][z] == 0) addFace(RIGHT_FACE,  glm::vec3(x, y, z), glm::vec3(1.0f, 0.0f, 0.0f), grassBlock.TexCoords[RIGHT_TEXTURE]);
						if (chunkData[x][y + 1][z] == 0) addFace(TOP_FACE,    glm::vec3(x, y, z), glm::vec3(0.0f, 1.0f, 0.0f), grassBlock.TexCoords[TOP_TEXTURE]);
						if (chunkData[x][y][z + 1] == 0) addFace(FRONT_FACE,  glm::vec3(x, y, z), glm::vec3(0.0f, 0.0f, 1.0f), grassBlock.TexCoords[FRONT_TEXTURE]);
					//Negative
						if (x - 1 > 0 and chunkData[x - 1][y][z] == 0) addFace(LEFT_FACE,   glm::vec3(x, y, z), glm::vec3(-1.0f, 0.0f, 0.0f), grassBlock.TexCoords[LEFT_TEXTURE]);
						if (y - 1 > 0 and chunkData[x][y - 1][z] == 0) addFace(BOTTOM_FACE, glm::vec3(x, y, z), glm::vec3(0.0f, -1.0f, 0.0f), grassBlock.TexCoords[BOTTOM_TEXTURE]);
						if (z - 1 > 0 and chunkData[x][y][z - 1] == 0) addFace(BACK_FACE,   glm::vec3(x, y, z), glm::vec3(0.0f, 0.0f, -1.0f), grassBlock.TexCoords[BACK_TEXTURE]);
					}
				}
		EndBatch();
		Flush();
	}

private:// Functions

	void ResetMesh() {for (uint32_t i = 0; i < MaxVertexCount; i++) chunkMesh[i] = gl::Vertex(glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f));}

	void BeginBatch() { offset = 0; }

	void EndBatch() { chunkMeshBuffer.Update(0, sizeof(chunkMesh), chunkMesh); }

	void Flush() {
		chunkMeshBuffer.Bind();
		chunkIndexBuffer.Bind();
		glDrawElements(GL_TRIANGLES, IndexCount, GL_UNSIGNED_INT, nullptr);

		IndexCount = 0;
	}

	void addFace(const Face& face, const glm::vec3& pos, const glm::vec3& Normal, const glm::vec2& Coords) {
		if (IndexCount >= MaxIndexCount) {
			EndBatch();
			Flush();
			BeginBatch();
		}

		for (uint8_t i = 0; i < 4; i++) chunkMesh[i + offset] = gl::Vertex(face[i] + pos, blockAtlas.GetSpriteCoords(Coords, {32,32})[i], Normal);

		IndexCount += 6;
		offset += 4;
	}
private:// Variables
	gl::VertexBuffer chunkMeshBuffer;
	gl::IndexBuffer chunkIndexBuffer;
	gl::Vertex chunkMesh[MaxVertexCount];

	uint32_t IndexCount = 0;
	uint32_t offset = 0;
};
}