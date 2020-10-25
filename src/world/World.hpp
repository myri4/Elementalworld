#pragma once

#include "Chunk.hpp"
#include "Block.hpp"
#include "../entityes/Player.hpp"

namespace wc {

	struct NoiseOptions {
		int octaves;
		float amplitude;
		float smoothness;
		float roughness;
		float offset;
	};

	float getNoiseFor(const glm::vec2& pos, const glm::vec2& chunkPosition, const NoiseOptions& options, const int& seed = 10) {
		int voxelX = pos.x + chunkPosition.x * chunkSize;
		int voxelZ = pos.y + chunkPosition.y * chunkSize;

		float value = 0.0f;
		value = glm::simplex(glm::vec3(voxelX / 32.0f, voxelZ / 32.0f, seed));

		value = (value + 1) / 2;

		return value *= 5;
	}

	class World : public NonCopyable {
	public:
		gl::Texture blockAtlas;
		std::unordered_map<int, Block> blockData;
		std::array<Chunk, 3> world;
		gl::IndexBuffer chunkIndexBuffer;
		
		World(){
		
		}
		~World() {

		}

		void Create() {
			worldShader.Create("shaderpacks/default/core.glsl");
			worldShader.use();
			worldShader.setInt("u_Texture", 0);

			LoadBlocks();
			LoadEnivoirment();
			SetUpIndexBuffer();

			WC_INFO("Sizeof chunk in bytes: {0}", sizeof(Chunk));

			skybox.Create("scripts/skybox.lua");
			for (int i = 0; i < world.size(); i++) {
				world[i].Create(i);
		
				for (uint8_t z = 0; z < chunkSize; z++) 
					for (uint8_t x = 0; x < chunkSize; x++) {
						int heightMap = getNoiseFor(glm::vec2(x, z), glm::vec2(to3D(world[i].chunkPosition).x, to3D(world[i].chunkPosition).z), worldNoiseOptions);
						for (int y = 0; y < chunkSize; y++)
							if ((int)to3D(world[i].chunkPosition).y * chunkSize + y == heightMap) { setBlock(glm::vec3(x, y, z), 1, i); }
					}
					UpdateWorldMesh(i);
			}
		}

		void Update(Player &p) {

			// activate shader
			worldShader.use();
			worldShader.setVec3("viewPos", p.Position);

			// pass projection matrix to shader (note that in this case it could change every frame)
			worldShader.setMat4("projection", p.projection);
			blockAtlas.Bind();

			// camera/view transformation
			worldShader.setMat4("view", p.GetView());
			Draw();
			skybox.Draw(glm::mat4(glm::mat3(p.GetView())), p.projection);
		}

	private:

		void SetUpIndexBuffer() {
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
		}

		void setBlock(const glm::vec3& pos, const int& block, const int& chunk) { // TEMP:
			if (pos.x > chunkSize || pos.y > chunkSize || pos.z > chunkSize) return;
			int x = pos.x;
			int y = pos.y;
			int z = pos.z;
			world[chunk].chunkData[to1D(glm::vec3(x, y, z))] = block;
		}

		void Draw() {
			for (uint32_t i = 0; i < world.size(); i++) {
				if (world[i].chunkMeshBuffer.GetVAO() == 0 || chunkIndexBuffer.GetEBO() == 0) return;
				// calculate the model matrix for each object and pass it to shader before drawing
				worldShader.setMat4("model", glm::translate(glm::mat4(1.0f), to3D(world[i].chunkPosition) * glm::vec3(chunkSize)));
				//std::async(std::launch::async, UpdateWorldMesh);
				UpdateWorldMesh(i);
			}
		}

		void Flush(const int& chunk) {
			chunkIndexBuffer.Bind();
			world[chunk].chunkMeshBuffer.Bind();
			glDrawElements(GL_TRIANGLES, world[chunk].IndexCount, GL_UNSIGNED_INT, nullptr);

			world[chunk].IndexCount = 0;
		}

		void UpdateWorldMesh(const int& chunk) {
				ResetMesh(chunk);
				BeginBatch(chunk);
				for (uint8_t y = 0; y < chunkSize; y++)
					for (uint8_t z = 0; z < chunkSize; z++)
						for (uint8_t x = 0; x < chunkSize; x++)
						{
							if (world[chunk].chunkData[to1D(glm::vec3(x, y, z))] > 0)
							{
								//Positive
								if (y + 1 < chunkSize) if (world[chunk].chunkData[to1D(glm::vec3(x, y + 1, z))] == 0) addFace(TOP_FACE,   glm::vec3(x, y, z), glm::vec3( 0.0f, 1.0f, 0.0f), blockData[world[chunk].chunkData[to1D(glm::vec3(x, y, z))]].TexCoords[(int)BlockTexture::TOP], chunk); else {};
								if (z + 1 < chunkSize) if (world[chunk].chunkData[to1D(glm::vec3(x, y, z + 1))] == 0) addFace(FRONT_FACE, glm::vec3(x, y, z), glm::vec3( 0.0f, 0.0f, 1.0f), blockData[world[chunk].chunkData[to1D(glm::vec3(x, y, z))]].TexCoords[(int)BlockTexture::FRONT], chunk); else {};
								if (x + 1 < chunkSize) if (world[chunk].chunkData[to1D(glm::vec3(x + 1, y, z))] == 0) addFace(RIGHT_FACE, glm::vec3(x, y, z), glm::vec3( 1.0f, 0.0f, 0.0f), blockData[world[chunk].chunkData[to1D(glm::vec3(x, y, z))]].TexCoords[(int)BlockTexture::RIGHT], chunk); else {};
								//Negative	   																																		 
								if (y - 1 > 0)		   if (world[chunk].chunkData[to1D(glm::vec3(x, y - 1, z))] == 0) addFace(BOTTOM_FACE,glm::vec3(x, y, z), glm::vec3( 0.0f,-1.0f, 0.0f), blockData[world[chunk].chunkData[to1D(glm::vec3(x, y, z))]].TexCoords[(int)BlockTexture::BOTTOM], chunk); else {};
								if (z - 1 > 0)		   if (world[chunk].chunkData[to1D(glm::vec3(x, y, z - 1))] == 0) addFace(BACK_FACE,  glm::vec3(x, y, z), glm::vec3( 0.0f, 0.0f,-1.0f), blockData[world[chunk].chunkData[to1D(glm::vec3(x, y, z))]].TexCoords[(int)BlockTexture::BACK], chunk); else {};
								if (x - 1 > 0)		   if (world[chunk].chunkData[to1D(glm::vec3(x - 1, y, z))] == 0) addFace(LEFT_FACE,  glm::vec3(x, y, z), glm::vec3(-1.0f, 0.0f, 0.0f), blockData[world[chunk].chunkData[to1D(glm::vec3(x, y, z))]].TexCoords[(int)BlockTexture::LEFT], chunk); else {};
							}
						}
				EndBatch(chunk);
				Flush(chunk);
		}

		void ResetMesh(const int& chunk) { for (uint32_t i = 0; i < MaxVertexCount; i++) world[chunk].chunkMesh[i] = gl::Vertex(glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f)); }

		void addFace(const Face& face, const glm::vec3& pos, const glm::vec3& Normal, const glm::vec2& Coords, const int& chunk) {
			if (world[chunk].IndexCount >= MaxIndexCount) {
				EndBatch(chunk);
				Flush(chunk);
				BeginBatch(chunk);
			}

			for (uint8_t i = 0; i < 4; i++) world[chunk].chunkMesh[i + world[chunk].offset] = gl::Vertex(face[i] + pos, blockAtlas.GetSpriteCoords(Coords, { 32,32 })[i], Normal);

			world[chunk].IndexCount += 6;
			world[chunk].offset += 4;
		}

		void BeginBatch(const int& chunk) { world[chunk].offset = 0; }

		void EndBatch(const int& chunk) { world[chunk].chunkMeshBuffer.Update(0, sizeof(world[chunk].chunkMesh), world[chunk].chunkMesh); }

		void LoadBlocks() {
			blockData.reserve(3);
			blockAtlas.load("assets/textures/block/blockAtlas.png");

			AddBlock("scripts/grassblock.lua", { 
				glm::vec2(0, 1),
				glm::vec2(1, 0),
				glm::vec2(0, 0),
				glm::vec2(0, 0),
				glm::vec2(0, 0),
				glm::vec2(0, 0)});
		}

		void AddBlock(const char* script, const std::array<glm::vec2, 6>& texCoords) {
			Block block;
			block.Create(script);
			block.material.ambient = glm::vec3(1.0f, 1.0f, 1.0f);
			block.material.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
			block.material.specular = glm::vec3(1.0f, 1.0f, 1.0f);
			block.material.shininess = 32.0f;
			block.SendMaterialToShader(worldShader, "material");
			block.TexCoords[(int)BlockTexture::TOP] = texCoords[(int)BlockTexture::TOP];
			block.TexCoords[(int)BlockTexture::BOTTOM] = texCoords[(int)BlockTexture::BOTTOM];
			block.TexCoords[(int)BlockTexture::LEFT] = texCoords[(int)BlockTexture::LEFT];
			block.TexCoords[(int)BlockTexture::RIGHT] = texCoords[(int)BlockTexture::RIGHT];
			block.TexCoords[(int)BlockTexture::FRONT] = texCoords[(int)BlockTexture::FRONT];
			block.TexCoords[(int)BlockTexture::BACK] = texCoords[(int)BlockTexture::BACK];
			blockData[block.id] = block;
		}

		void LoadEnivoirment() {
			skybox.Create("scripts/skybox.lua");
		}
		gl::Shader worldShader;
		gl::Skybox skybox;
		NoiseOptions worldNoiseOptions;
	};

}