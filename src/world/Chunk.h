#pragma once

#include <wclibs/wclibspch.h>

#include "../Block.h"

wc::Block grassBlock;

static const size_t chunkSize = 32;

class Vertex {
public:
	glm::vec3 Position = {0,0,0};
	glm::vec2 TexCoords = {0,0};
	float TexID = 0;
	Vertex(){}
	Vertex(const Vertex& vertex) : Position(vertex.Position), TexCoords(vertex.TexCoords), TexID(vertex.TexID){}
	Vertex(glm::vec3 pos, glm::vec2 texCoord, float texID) : Position(pos), TexCoords(texCoord), TexID(texID) {}
	~Vertex(){}
};

using Face = std::array < glm::vec3, 4 >;

Face BACK_FACE = {
	glm::vec3( 0.5f,  0.5f, -0.5f), // top-right
	glm::vec3(-0.5f,  0.5f, -0.5f), // top-left
	glm::vec3(-0.5f, -0.5f, -0.5f), // Bottom-left
	glm::vec3( 0.5f, -0.5f, -0.5f), // bottom-right    
};
Face FRONT_FACE = {
	glm::vec3( 0.5f,  0.5f,  0.5f), // top-right
	glm::vec3( 0.5f, -0.5f,  0.5f), // bottom-right        
	glm::vec3(-0.5f, -0.5f,  0.5f), // bottom-left
	glm::vec3(-0.5f,  0.5f,  0.5f)  // top-left   
};
Face LEFT_FACE = {
	glm::vec3(-0.5f,  0.5f,  0.5f),  // top-right
	glm::vec3(-0.5f, -0.5f,  0.5f),  // bottom-right
	glm::vec3(-0.5f, -0.5f, -0.5f),  // bottom-left
	glm::vec3(-0.5f,  0.5f, -0.5f)   // top-left   
};
Face RIGHT_FACE = {
	 glm::vec3(0.5f,  0.5f, -0.5f),  // top-right      
	 glm::vec3(0.5f, -0.5f, -0.5f),  // bottom-right          
	 glm::vec3(0.5f, -0.5f,  0.5f),  // bottom-left
	 glm::vec3(0.5f,  0.5f,  0.5f)   // top-left
};
Face BOTTOM_FACE = {
	glm::vec3(-0.5f, -0.5f, -0.5f),  // top-right
	glm::vec3(-0.5f, -0.5f,  0.5f),  // bottom-right
	glm::vec3( 0.5f, -0.5f,  0.5f),  // bottom-left
	glm::vec3( 0.5f, -0.5f, -0.5f)   // top-left  
};
Face TOP_FACE = {
	glm::vec3( 0.5f,  0.5f, -0.5f), // top-right
	glm::vec3( 0.5f,  0.5f,  0.5f), // bottom-right                 
	glm::vec3(-0.5f,  0.5f,  0.5f), // bottom-left  
	glm::vec3(-0.5f,  0.5f, -0.5f)  // top-left 
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
public: // Functions 
	Chunk() {}
	Chunk(glm::vec3 pos) { Create(pos); }
	~Chunk() {}
	void Create(glm::vec3 pos) {
		//Temp
		int limit = 10;
		for (int x = 0; x < limit; x++)
			for (int y = 0; y < limit; y++)
				for (int z = 0; z < limit; z++)chunkData[x + 1][y + 1][z + 1] = 1;
		//----

		chunkMesh.reserve(MaxVertexCount);

		chunkPosition = pos;

		chunkMeshBuffer.Create(nullptr, MaxVertexCount * sizeof(Vertex), GL_DYNAMIC_DRAW);
		gl::VertexAttribPointer(0, 3, sizeof(Vertex), (void*)offsetof(Vertex, Position));  // position attribute
		gl::VertexAttribPointer(1, 2, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords)); // texture coord attribute
		gl::VertexAttribPointer(2, 1, sizeof(Vertex), (void*)offsetof(Vertex, TexID));     // Texture ID attribute

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
		chunkMesh.clear();
		for (int32_t x = 0; x < chunkSize; x++)
			for (int32_t y = 0; y < chunkSize; y++)
				for (int32_t z = 0; z < chunkSize; z++) {
					if (chunkData[x][y][z] > 0) {
		
						if (chunkData[x + 1][y][z] == 0) addFace(RIGHT_FACE,  { x,y,z }, 2);
						if (chunkData[x - 1][y][z] == 0) addFace(LEFT_FACE,   { x,y,z }, 2);
		
						if (chunkData[x][y + 1][z] == 0) addFace(TOP_FACE,	  { x,y,z }, 2);
						if (chunkData[x][y - 1][z] == 0) addFace(BOTTOM_FACE, { x,y,z }, 2);
		
						if (chunkData[x][y][z + 1] == 0) addFace(FRONT_FACE,  { x,y,z }, 2);
						if (chunkData[x][y][z - 1] == 0) addFace(BACK_FACE,   { x,y,z }, 2);
					}
				}
		EndBatch();
	}
	void Draw(gl::Shader& shader) {
		
		/*if (IndexCount > MaxIndexCount || TextureSlotIndex > MaxTextures) {
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

private:// Functions

	/*
	void DrawQuad(uint32_t texID){
	
	}
	*/

	void BeginBatch() {

	}

	void EndBatch() {
		//chunkMeshBuffer.Update(0, sizeof(mesh), mesh);
		chunkMeshBuffer.Update(0, chunkMesh.size() * sizeof(Vertex), &chunkMesh[0]);
	}

	void Flush() {
		//for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++) glBindTextureUnit(i, s_Data.TextureSlots[i]);

		glBindTextureUnit(1, grassBlock.blockTexture[0].GetRendererID());
		glBindTextureUnit(2, grassBlock.blockTexture[1].GetRendererID());

		chunkMeshBuffer.Bind();
		chunkIndexBuffer.Bind();
		glDrawElements(GL_TRIANGLES, IndexCount, GL_UNSIGNED_INT, nullptr);
	}



	void addFace(Face& face, glm::vec3 pos, float TexID) {
		for (int i = 0; i < 4; i++) {
			chunkMesh.emplace_back(Vertex(face[i] + pos, TexCoords[i], TexID));
			mesh[i + offset] = Vertex(face[i] + pos, TexCoords[i], TexID);
			IndexCount += 3;
		}
		offset += 1;
	}
private:// Variables
	gl::VertexBuffer chunkMeshBuffer;
	gl::IndexBuffer chunkIndexBuffer;
	std::vector<Vertex> chunkMesh;

	uint32_t IndexCount = 0;
	uint32_t offset = 0;
	Vertex mesh[MaxFaceCount];

	std::array<uint32_t, MaxTextures> TextureSlots;
	uint32_t TextureSlotIndex = 1;
};