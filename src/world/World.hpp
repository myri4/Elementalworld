#pragma once

#include "Chunk.hpp"
#include "../entityes/Player.hpp"

namespace wc {
		
	Block grassBlock;

	std::vector<Block> blockData;

	gl::Texture blockAtlas;

	struct NoiseOptions {
		int octaves;
		float amplitude;
		float smoothness;
		float roughness;
		float offset;
	};

	float getNoiseFor(const glm::vec2& pos, const glm::vec2& chunkPosition, const NoiseOptions& options, const int& seed = 0) {
		int voxelX = pos.x + chunkPosition.x * chunkSize;
		int voxelZ = pos.y + chunkPosition.y * chunkSize;

		float value = 0.0f;
		value = glm::simplex(glm::vec3(voxelX / 32.0f, voxelZ / 32.0f, seed));

		//for (int i = 0; i < options.octaves; i++) {}

		value = (value + 1) / 2;

		return value *= 5;
	}

	void AddBlock() {
		Block block;
		block.Create("scripts/grassblock.lua");
		blockData.push_back(block);
	}

	class World : public NonCopyable {
	public:
		//std::unordered_map<glm::vec3, Chunk> world;
		std::array<Chunk, 3> world;
		
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

			skybox.Create("scripts/skybox.lua");
			for (int i = 0; i < world.size(); i++) {
				world[i].Create(glm::vec3(i, 0, 0));
		
				for (uint8_t z = 0; z < chunkSize; z++) 
					for (uint8_t x = 0; x < chunkSize; x++) {
						int heightMap = getNoiseFor(glm::vec2(x, z), glm::vec2(world[i].chunkPosition.x, world[i].chunkPosition.z), worldNoiseOptions);
						for (int y = 0; y < chunkSize; y++)
							if ((int)world[i].chunkPosition.y * chunkSize + y == heightMap) { setBlock(glm::vec3(x, y, z), 1, i); }
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

		void setBlock(const glm::vec3& pos, const int& block, const int& chunk) { // TEMP:
			if (pos.x > chunkSize || pos.y > chunkSize || pos.z > chunkSize) return;
			int x = pos.x;
			int y = pos.y;
			int z = pos.z;
			world[chunk].chunkData[x][y][z] = block;
		}

		void Draw() {
			for (int i = 0; i < world.size(); i++) {
				if (world[i].chunkMeshBuffer.GetVAO() == 0 || world[i].chunkIndexBuffer.GetEBO() == 0) return;
				// calculate the model matrix for each object and pass it to shader before drawing
				worldShader.use();
				worldShader.setMat4("model", glm::translate(glm::mat4(1.0f), world[i].chunkPosition * glm::vec3(chunkSize)));
				UpdateWorldMesh(i);
			}
		}

		void Flush(const int& chunk) {
			world[chunk].chunkMeshBuffer.Bind();
			world[chunk].chunkIndexBuffer.Bind();
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
							if (world[chunk].chunkData[x][y][z] > 0) {
								//Positive
								if (world[chunk].chunkData[x + 1][y][z] == 0) addFace(RIGHT_FACE, glm::vec3(x, y, z), glm::vec3(1.0f, 0.0f, 0.0f), grassBlock.TexCoords[RIGHT_TEXTURE], chunk);
								if (world[chunk].chunkData[x][y + 1][z] == 0) addFace(TOP_FACE, glm::vec3(x, y, z), glm::vec3(0.0f, 1.0f, 0.0f), grassBlock.TexCoords[TOP_TEXTURE], chunk);
								if (world[chunk].chunkData[x][y][z + 1] == 0) addFace(FRONT_FACE, glm::vec3(x, y, z), glm::vec3(0.0f, 0.0f, 1.0f), grassBlock.TexCoords[FRONT_TEXTURE], chunk);
								//Negative
								if (x - 1 > 0 and world[chunk].chunkData[x - 1][y][z] == 0) addFace(LEFT_FACE, glm::vec3(x, y, z), glm::vec3(-1.0f, 0.0f, 0.0f), grassBlock.TexCoords[LEFT_TEXTURE], chunk);
								if (y - 1 > 0 and world[chunk].chunkData[x][y - 1][z] == 0) addFace(BOTTOM_FACE, glm::vec3(x, y, z), glm::vec3(0.0f, -1.0f, 0.0f), grassBlock.TexCoords[BOTTOM_TEXTURE], chunk);
								if (z - 1 > 0 and world[chunk].chunkData[x][y][z - 1] == 0) addFace(BACK_FACE, glm::vec3(x, y, z), glm::vec3(0.0f, 0.0f, -1.0f), grassBlock.TexCoords[BACK_TEXTURE], chunk);
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
			blockAtlas.load("assets/textures/block/blockAtlas.png");
			grassBlock.Create("scripts/grassblock.lua");
			grassBlock.material.ambient = glm::vec3(1.0f, 1.0f, 1.0f);
			grassBlock.material.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
			grassBlock.material.specular = glm::vec3(1.0f, 1.0f, 1.0f);
			grassBlock.material.shininess = 32.0f;
			grassBlock.SendMaterialToShader(worldShader, "material");
			grassBlock.TexCoords[TOP_TEXTURE] = {0, 1};
			grassBlock.TexCoords[BOTTOM_TEXTURE] = { 1, 0 };
			grassBlock.TexCoords[LEFT_TEXTURE] = { 0, 0 };
			grassBlock.TexCoords[RIGHT_TEXTURE] = { 0, 0 };
			grassBlock.TexCoords[FRONT_TEXTURE] = { 0, 0 };
			grassBlock.TexCoords[BACK_TEXTURE] = { 0, 0 };
		}

		void LoadEnivoirment() {
			skybox.Create("scripts/skybox.lua");
		}
		gl::Shader worldShader;
		gl::Skybox skybox;
		NoiseOptions worldNoiseOptions;
	};

}