#pragma once

#include <wclibs/pch.hpp>

#include "../Block.h"

wc::Block grassBlock;

static const size_t chunkSize = 32;
static const size_t chunkVolume = chunkSize * chunkSize * chunkSize;
static const float blockSize = 0.5f;

using Face = std::array < glm::vec3, 4 >;

Face BACK_FACE = {
	glm::vec3( blockSize,  blockSize, -blockSize), // top-right
	glm::vec3(-blockSize,  blockSize, -blockSize), // top-left
	glm::vec3(-blockSize, -blockSize, -blockSize), // Bottom-left  
	glm::vec3( blockSize, -blockSize, -blockSize), // bottom-right 
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
	glm::vec2(1.0f, 1.0f),
	glm::vec2(1.0f, 0.0f),
	glm::vec2(0.0f, 0.0f),
	glm::vec2(0.0f, 1.0f),
};

class Chunk {
public: // Variables
	glm::vec3 chunkPosition = { 0,0,0 };
	int chunkData[chunkSize][chunkSize][chunkSize];
	bool used = false;
	uint32_t IndexCount = 0;
public: // Functions 
	Chunk() {}
	Chunk(const glm::vec3& pos) { Create(pos); }
	~Chunk() {}
	void Create(const glm::vec3& pos) {
		//Temp
		chunkPosition = pos;
		for (int x = 1; x < 16; x++)
			for (int y = 1; y < 16; y++)setBlock({ x,y,1 }, 1);

		chunkMeshBuffer.Create(nullptr, MaxVertexCount * sizeof(gl::Vertex), GL_DYNAMIC_DRAW);
		gl::VertexAttribPointer(0, 3, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, Position));  // position attribute
		gl::VertexAttribPointer(1, 2, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, TexCoords)); // texture coord attribute
		gl::VertexAttribPointer(2, 1, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, TexID));     // Texture ID attribute
		gl::VertexAttribPointer(3, 3, sizeof(gl::Vertex), (void*)offsetof(gl::Vertex, Normal));     // Normal attribute


		uint32_t indices[MaxIndexCount];
		uint32_t offset = 0;
		for (int i = 0; i < MaxIndexCount; i += 6) {
			indices[i + 0] = 0 + offset;
			indices[i + 1] = 1 + offset;
			indices[i + 2] = 2 + offset;

			indices[i + 3] = 2 + offset;
			indices[i + 4] = 3 + offset;
			indices[i + 5] = 0 + offset;

			offset += 4;
		}
		chunkIndexBuffer.Create(indices, MaxIndexCount);

		UpdateMesh();
	}
	void Update() {

	}
	void UpdateMesh() {
			for (int32_t x = 0; x < chunkSize; x++) 
			for (int32_t z = 0; z < chunkSize; z++)
			for (int32_t y = 0; y < chunkSize; y++)
				{
					if (chunkData[x][y][z] > 0) {

						if (chunkData[x + 1][y][z] == 0) addFace(RIGHT_FACE, { x,y,z },  glm::vec3( 1.0f, 0.0f, 0.0f), 2);
						if (chunkData[x - 1][y][z] == 0) addFace(LEFT_FACE, { x,y,z },   glm::vec3(-1.0f, 0.0f, 0.0f), 2);
																									  
						if (chunkData[x][y + 1][z] == 0) addFace(TOP_FACE, { x,y,z },    glm::vec3( 0.0f, 1.0f, 0.0f), 2);
						if (chunkData[x][y - 1][z] == 0) addFace(BOTTOM_FACE, { x,y,z }, glm::vec3( 0.0f,-1.0f, 0.0f), 2);

						if (chunkData[x][y][z + 1] == 0) addFace(FRONT_FACE, { x,y,z },  glm::vec3( 0.0f, 0.0f, 1.0f), 2);
						if (chunkData[x][y][z - 1] == 0) addFace(BACK_FACE, { x,y,z },   glm::vec3( 0.0f, 0.0f,-1.0f), 2);
					}
				}
		EndBatch();
	}
	void Draw(gl::Shader& shader) {

		/*if (IndexCount > MaxIndexCount || TextureSlotIndex > GetMaxTextures()) {
			EndBatch();
			Flush();
			BeginBatch();
		}*/

		shader.use();
		// calculate the model matrix for each object and pass it to shader before drawing
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, chunkPosition);
		shader.setMat4("model", model);
		Flush();
	}
	void setBlock(glm::vec3 pos, int block) {
		int x = pos.x;
		int y = pos.y;
		int z = pos.z;
		chunkData[x][y][z] = block;
	}
private:// Functions

	/*
	void DrawQuad(uint32_t texID){

	}
	*/

	void BeginBatch() {

	}

	void EndBatch() {
		chunkMeshBuffer.Update(0, sizeof(chunkMesh), chunkMesh);
	}

	void Flush() {
		//for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++) glBindTextureUnit(i, s_Data.TextureSlots[i]);

		glBindTextureUnit(1, grassBlock.blockTexture[0].GetRendererID());
		glBindTextureUnit(2, grassBlock.blockTexture[1].GetRendererID());

		chunkMeshBuffer.Bind();
		chunkIndexBuffer.Bind();
		glDrawElements(GL_TRIANGLES, IndexCount, GL_UNSIGNED_INT, nullptr);
	}



	void addFace(const Face& face, const glm::vec3& pos, const glm::vec3 Normal, const float& TexID) {
		for (int i = 0; i < 4; i++) {
			chunkMesh[i + offset] = gl::Vertex(face[i] + pos, TexCoords[i], Normal, TexID);
		}

		offset += 4;
		IndexCount += 6;
	}
private:// Variables
	gl::VertexBuffer chunkMeshBuffer;
	gl::IndexBuffer chunkIndexBuffer;
	gl::Vertex chunkMesh[MaxVertexCount];
	uint32_t offset = 0;


	std::array<uint32_t, MaxTextures> TextureSlots;
	uint32_t TextureSlotIndex = 1;
};