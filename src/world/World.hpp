#pragma once

#include "Chunk.hpp"
#include "Block.hpp"
#include "../entityes/Player.hpp"
#include <Utilitiess/Time.hpp>

namespace wc {

	struct NoiseOptions {
		int octaves = 0;
		float amplitude = 0.0f;
		float smoothness = 0.0f;
		float roughness = 0.0f;
		float offset = 0.0f;
	};

	float getNoiseFor(const glm::vec2& pos, const glm::vec2& chunkPosition, const NoiseOptions& options, const int& seed = 0) {
		int voxelX = pos.x + chunkPosition.x * chunkSize;
		int voxelZ = pos.y + chunkPosition.y * chunkSize;

		float value = 0.0f;
		value = glm::simplex(glm::vec3(voxelX / 64.0f, voxelZ / 64.0f, seed));

		value = (value + 1) / 2;

		return value *= 32 + 32;
	}


	class World : public NonCopyable {
	public:
		gl::Texture blockAtlas;
		std::unordered_map<int,Block> blockData;
		std::array<Chunk, chunkSize* chunkSize* chunkSize> world;
		gl::IndexBuffer chunkIndexBuffer;
		Player p;

		World() {

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

			for (int i = 0; i < world.size(); i++) {
				world[i].Create(i);

				for (uint8_t z = 0; z < chunkSize; z++)
					for (uint8_t x = 0; x < chunkSize; x++) {
						int heightMap = getNoiseFor(glm::vec2(x, z), glm::vec2(to3D(world[i].chunkPosition).x, to3D(world[i].chunkPosition).z), worldNoiseOptions);
						for (int y = 0; y < chunkSize; y++) {
							if ((int)to3D(world[i].chunkPosition).y * chunkSize + y == heightMap) { setBlock(glm::vec3(x, y, z), 1, i); }
						}
					}
				UpdateWorldMesh(i);
			}
			p.InitPlayer({ 8,50,0 });
		}

		void Update(sf::RenderWindow& window, const bool& CenterMouse, const float& deltaTime) {
			p.UpdatePlayer({ window.getPosition().x, window.getPosition().y }, CenterMouse, deltaTime);

			// activate shader
			worldShader.use();
			worldShader.setVec3("viewPos", p.Position);

			// pass projection matrix to shader (note that in this case it could change every frame)
			worldShader.setMat4("projection", p.projection);
			blockAtlas.Bind();

			// camera/view transformation
			worldShader.setMat4("view", p.GetView());
			Draw();
			if (!Fog) 
				skybox.Draw(glm::mat4(glm::mat3(p.GetView())), p.projection);

			
		}

		void OnEvent(const float& deltaTime) {
			p.UpdatePlayerInput(deltaTime);
		}

	private:

		void Draw() {
			for (uint32_t i = 0; i < world.size(); i++) {
				if (world[i].chunkMeshBuffer.GetVAO() == 0 || chunkIndexBuffer.GetEBO() == 0) return;
				// calculate the model matrix for each object and pass it to shader before drawing
				worldShader.setMat4("model", glm::translate(glm::mat4(1.0f), to3D(world[i].chunkPosition) * glm::vec3(chunkSize)));
				//std::async(std::launch::async, UpdateWorldMesh);
				Flush(i);
			}
		}

		void SetUpIndexBuffer() {
			uint32_t indices[MaxFaceCount * 6];
			uint32_t offset = 0;
			for (uint32_t i = 0; i < MaxFaceCount * 6; i += 6) {
				indices[i + 0] = 0 + offset;
				indices[i + 1] = 1 + offset;
				indices[i + 2] = 2 + offset;

				indices[i + 3] = 2 + offset;
				indices[i + 4] = 3 + offset;
				indices[i + 5] = 0 + offset;

				offset += 4;
			}
			chunkIndexBuffer.Create(indices, sizeof(indices));
		}

		void setBlock(const glm::vec3& pos, const int& block, const int& chunk) {
			if (pos.x > chunkSize || pos.y > chunkSize || pos.z > chunkSize) return;
			int x = pos.x;
			int y = pos.y;
			int z = pos.z;
			world[chunk].chunkData[x][y][z] = block;
		}

		void Flush(const int& chunk) {
			if (world[chunk].IndexCount == 0) return;
			chunkIndexBuffer.Bind();
			world[chunk].chunkMeshBuffer.Bind();
			glDrawElements(GL_TRIANGLES, world[chunk].IndexCount, GL_UNSIGNED_INT, nullptr);
		}

		void UpdateWorldMesh(const int& chunk) {
			//for (uint32_t i = 0; i < MaxVertexCount; i++) world[chunk].chunkMesh[i] = gl::Vertex(glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f)); // Reseting the mesh
			world[chunk].offset = 0; // BeginBatch();
			for (int8_t y = 0; y < chunkSize; y++)
				for (int8_t z = 0; z < chunkSize; z++)
					for (int8_t x = 0; x < chunkSize; x++)
					{
						//if (y + 1 < chunkSize && z + 1 < chunkSize && x + 1 < chunkSize) {
						if (world[chunk].chunkData[x][y][z] > 0)
						{
							//Positive
							if (y + 1 < chunkSize) { if (world[chunk].chunkData[x][y + 1][z] == 0) addFace(TOP_FACE, glm::vec3(x, y, z), glm::vec3(0.0f, 1.0f, 0.0f), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::TOP], chunk); }
							//else 
							//	{if (chunk + to1D({ 0,1,0 }) < world.size()) if (world[chunk + to1D({ 0,1,0 })].chunkData[x][0][z] == 0) addFace(TOP_FACE,   glm::vec3(x, y, z), glm::vec3( 0.0f,  1.0f,  0.0f), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::TOP],   chunk);}
							
							if (z + 1 < chunkSize) { if (world[chunk].chunkData[x][y][z + 1] == 0) addFace(FRONT_FACE, glm::vec3(x, y, z), glm::vec3(0.0f, 0.0f, 1.0f), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::FRONT], chunk); }
							//else
							//	{if (chunk + to1D({ 0,0,1 }) < world.size()) if(world[chunk + to1D({ 0,0,1 })].chunkData[x][y][0] == 0) addFace(FRONT_FACE, glm::vec3(x, y, z), glm::vec3( 0.0f,  0.0f,  1.0f), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::FRONT], chunk);}
							
							if (x + 1 < chunkSize) { if (world[chunk].chunkData[x + 1][y][z] == 0) addFace(RIGHT_FACE, glm::vec3(x, y, z), glm::vec3(1.0f, 0.0f, 0.0f), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::RIGHT], chunk); }
							//else
							//	{if (chunk + to1D({ 1,0,0 }) < world.size()) if (world[chunk + to1D({ 1,0,0 })].chunkData[0][y][z] == 0) addFace(RIGHT_FACE, glm::vec3(x, y, z), glm::vec3( 1.0f,  0.0f,  0.0f), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::RIGHT], chunk);}
							
							//Negative	   																										 					  																						 
							if (y - 1 > 0) { if (world[chunk].chunkData[x][y - 1][z] == 0) addFace(BOTTOM_FACE, glm::vec3(x, y, z), glm::vec3(0.0f, -1.0f, 0.0f), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::BOTTOM], chunk); }
							//else
							//	{if (chunk - to1D({ 0,1,0 }) > 0) if (world[chunk - to1D({ 0,1,0 })].chunkData[x][chunkSize][z] == 0) addFace(BOTTOM_FACE,glm::vec3(x, y, z), glm::vec3( 0.0f, -1.0f,  0.0f), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::BOTTOM],chunk);}
							
							if (z - 1 > 0) { if (world[chunk].chunkData[x][y][z - 1] == 0) addFace(BACK_FACE, glm::vec3(x, y, z), glm::vec3(0.0f, 0.0f, -1.0f), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::BACK], chunk);}
							//else
							//	{if (chunk - to1D({ 0,0,1 }) > 0) if (world[chunk - to1D({ 0,0,1 })].chunkData[x][y][chunkSize] == 0) addFace(BACK_FACE,  glm::vec3(x, y, z), glm::vec3( 0.0f,  0.0f, -1.0f), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::BACK],  chunk);}
							
							if (x - 1 > 0) { if (world[chunk].chunkData[x - 1][y][z] == 0) addFace(LEFT_FACE, glm::vec3(x, y, z), glm::vec3(-1.0f, 0.0f, 0.0f), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::LEFT], chunk);}
							//else
							//	{if (chunk - to1D({ 1,0,0 }) > 0) if (world[chunk - to1D({ 1,0,0 })].chunkData[chunkSize][x][z] == 0) addFace(LEFT_FACE, glm::vec3(x, y, z), glm::vec3(-1.0f, 0.0f, 0.0f), blockData[world[chunk].chunkData[x][y][z]].TexCoords[(int)BlockTexture::LEFT], chunk);}
						}
						//}
					}
			world[chunk].chunkMeshBuffer.Update(0, sizeof(world[chunk].chunkMesh), world[chunk].chunkMesh); // EndBatch();
			Flush(chunk);
		}

		void addFace(const Face& face, const glm::vec3& pos, const glm::vec3& Normal, const glm::vec2& Coords, const int& chunk) {
			if (world[chunk].IndexCount >= MaxFaceCount * 6) {
				world[chunk].chunkMeshBuffer.Update(0, sizeof(world[chunk].chunkMesh), world[chunk].chunkMesh); // EndBatch();
				Flush(chunk);
				WC_ERROR("Memory overflow!");
				world[chunk].offset = 0;// BeginBatch();
			}

			for (uint8_t i = 0; i < 4; i++) world[chunk].chunkMesh[i + world[chunk].offset] = gl::Vertex(face[i] + pos, blockAtlas.GetSpriteIndexCoords(Coords, { 32,32 })[i], Normal);

			world[chunk].IndexCount += 6;
			world[chunk].offset += 4;
		}

		void LoadBlocks() {
			blockData.reserve(3);
			blockAtlas.load("assets/textures/block/blockAtlas.png");

			AddBlock("scripts/grassblock.lua", {
				glm::vec2(0, 1),
				glm::vec2(1, 0),
				glm::vec2(0, 0),
				glm::vec2(0, 0),
				glm::vec2(0, 0),
				glm::vec2(0, 0) });
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
			glm::vec3 fogColor = glm::vec3(0.5f, 0.5f, 0.5f);
			worldShader.setBool("fog", Fog);
			worldShader.setVec3("fogColor", fogColor);
			if (Fog)
				glClearColor(fogColor.r, fogColor.g, fogColor.b, 1.0f);
			skybox.Create("scripts/skybox.lua", p.Far);
		}
		bool Fog = true;
		gl::Shader worldShader;
		gl::Skybox skybox;
		NoiseOptions worldNoiseOptions;
	};

}