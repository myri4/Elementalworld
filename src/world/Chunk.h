#pragma once

#include <wclibs/pch.hpp>

#include "../Block.h"

wc::Block grassBlock;

static const size_t chunkVolume = chunkSize * chunkSize * chunkSize;
static const float blockSize = 0.5f;
namespace wc {

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

glm::vec2 TexCoords[] = {
	glm::vec2(0.0f, 0.0f),
	glm::vec2(0.0f, 1.0f),
	glm::vec2(1.0f, 1.0f),
	glm::vec2(1.0f, 0.0f),
};

class Chunk {
public: // Variables
	glm::vec3 chunkPosition = { 0,0,0 };
	int chunkData[chunkSize][chunkSize][chunkSize];
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
		gl::VertexAttribPointer(2, 1, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, TexID));     // Texture ID attribute
		gl::VertexAttribPointer(3, 3, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, Normal));     // Normal attribute

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

		//Set a default value for the texture IDs
		for (size_t i = 0; i < MaxTextureUnits(); i++)
			TextureSlots[i] = 0;

		WC_INFO("Created chunk at X:{0} Y:{1} Z:{2}", pos.x, pos.y, pos.z);

		//for (uint16_t x = 2; x < chunkSize; x++)//5518
		//for (uint16_t z = 2; z < chunkSize; z++)
		//for (uint16_t y = 2; y < chunkSize; y++)
		setBlock({ 1,1,1 }, 1);

		UpdateMesh();
	}
	void Update() {

	}
	void UpdateMesh() {
		BeginBatch();
		for (uint8_t x = 0; x < chunkSize; x++)
			for (uint8_t z = 0; z < chunkSize; z++)
				for (uint8_t y = 0; y < chunkSize; y++)
				{
					if (chunkData[x][y][z] > 0) {
						if (chunkData[x + 1][y][z] == 0) addFace(RIGHT_FACE, { x,y,z }, glm::vec3(1.0f, 0.0f, 0.0f), grassBlock.blockTexture[RIGHT_TEXTURE].GetRendererID());
						if (chunkData[x - 1][y][z] == 0) addFace(LEFT_FACE, { x,y,z }, glm::vec3(-1.0f, 0.0f, 0.0f), grassBlock.blockTexture[LEFT_TEXTURE].GetRendererID());

						if (chunkData[x][y + 1][z] == 0) addFace(TOP_FACE, { x,y,z }, glm::vec3(0.0f, 1.0f, 0.0f), grassBlock.blockTexture[TOP_TEXTURE].GetRendererID());
						if (chunkData[x][y - 1][z] == 0) addFace(BOTTOM_FACE, { x,y,z }, glm::vec3(0.0f, -1.0f, 0.0f), grassBlock.blockTexture[BOTTOM_TEXTURE].GetRendererID());

						if (chunkData[x][y][z + 1] == 0) addFace(FRONT_FACE, { x,y,z }, glm::vec3(0.0f, 0.0f, 1.0f), grassBlock.blockTexture[FRONT_TEXTURE].GetRendererID());
						if (chunkData[x][y][z - 1] == 0) addFace(BACK_FACE, { x,y,z }, glm::vec3(0.0f, 0.0f, -1.0f), grassBlock.blockTexture[BACK_TEXTURE].GetRendererID());
					}
				}
		EndBatch();
		Flush();
	}
	void Draw(const gl::Shader& shader) {
		// calculate the model matrix for each object and pass it to shader before drawing
		shader.use();
		shader.setMat4("model", glm::translate(glm::mat4(1.0f), chunkPosition));
		UpdateMesh();
	}
	void setBlock(glm::vec3 pos, int block) {
		int x = pos.x;
		int y = pos.y;
		int z = pos.z;
		chunkData[x][y][z] = block;
	}
private:// Functions

	void BeginBatch() { offset = 0; }

	void EndBatch() { chunkMeshBuffer.Update(0, sizeof(chunkMesh), chunkMesh); }

	void Flush() {
		for (uint32_t i = 0; i < TextureSlotIndex; i++)
			glBindTextureUnit(i, TextureSlots[i]);

		chunkMeshBuffer.Bind();
		chunkIndexBuffer.Bind();
		glDrawElements(GL_TRIANGLES, IndexCount, GL_UNSIGNED_INT, nullptr);
		IndexCount = 0;
		TextureSlotIndex = 0;
	}

	void addFace(const Face& face, const glm::vec3& pos, const glm::vec3& Normal, const uint32_t& textureID) {
		if (IndexCount >= MaxIndexCount || TextureSlotIndex >= MaxTextureUnits()) {
			EndBatch();
			Flush();
			BeginBatch();
		}

		float textureIndex = 0.0f;
		for (uint32_t i = 0; i < TextureSlotIndex; i++) {
			if (TextureSlots[i] == textureID) {
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f) {
			textureIndex = (float)TextureSlotIndex;
			TextureSlots[TextureSlotIndex] = textureID;
			TextureSlotIndex++;
		}

		for (uint8_t i = 0; i < 4; i++) chunkMesh[i + offset] = gl::Vertex(face[i] + pos, TexCoords[i], Normal, textureIndex);

		IndexCount += 6;
		offset += 4;
	}
private:// Variables
	gl::VertexBuffer chunkMeshBuffer;
	gl::IndexBuffer chunkIndexBuffer;
	gl::Vertex chunkMesh[MaxVertexCount];

	uint32_t IndexCount = 0;
	uint32_t offset = 0;

	std::array<uint32_t, MaxTextures> TextureSlots;
	uint32_t TextureSlotIndex = 0;
};
}